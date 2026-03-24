"""
智能公共交通顾问程序

=== 实体和概念定义 ===

1. 站点 (Station)
   - 表示公共交通网络中的一个停靠点
   - 属性：名称、所在区域、设施列表

2. 区段 (Section)
   - 连接两个相邻站点的线路段
   - 属性：起点站、终点站、所属线路、成本、持续时间

3. 成本 (Cost)
   - 表示乘坐某区段或旅程的费用
   - 属性：金额、货币单位

4. 持续时间 (Duration)
   - 表示乘坐某区段或旅程所需时间
   - 属性：分钟数

5. 旅程 (Journey)
   - 从起点到终点的完整路线
   - 属性：起点站、终点站、途经区段列表、总成本、总持续时间、换乘次数

6. 偏好模式 (PreferenceMode)
   - 用户查询旅程时的优化偏好
   - 选项：最快、最便宜、最少换乘

=== 程序输入 ===
- 网络数据文件 (JSON 格式)
- 用户菜单选择
- 旅程查询参数 (起点、终点、偏好)

=== 程序输出 ===
- 站点列表
- 旅程详情 (路线、成本、时间)
- 网络摘要统计
- 操作确认信息

=== 程序不做的事情 ===
- 不提供实时地图或图形界面
- 不提供实时车辆位置追踪
- 不处理票务预订或支付
- 不提供天气或延误信息
- 不存储用户历史记录
"""

import json
from dataclasses import dataclass, field
from typing import List, Optional, Dict, Any
from enum import Enum


# ==================== 实体定义 ====================

class PreferenceMode(Enum):
    """偏好模式"""
    FASTEST = "最快"
    CHEAPEST = "最便宜"
    LEAST_TRANSFERS = "最少换乘"


@dataclass
class Cost:
    """成本"""
    amount: float
    currency: str = "CNY"

    def __add__(self, other: 'Cost') -> 'Cost':
        return Cost(self.amount + other.amount, self.currency)

    def __str__(self) -> str:
        return f"{self.amount:.2f} {self.currency}"


@dataclass
class Duration:
    """持续时间"""
    minutes: int

    def __add__(self, other: 'Duration') -> 'Duration':
        return Duration(self.minutes + other.minutes)

    def __str__(self) -> str:
        if self.minutes >= 60:
            hours = self.minutes // 60
            mins = self.minutes % 60
            return f"{hours}小时{mins}分钟" if mins > 0 else f"{hours}小时"
        return f"{self.minutes}分钟"


@dataclass
class Station:
    """站点"""
    name: str
    zone: str
    facilities: List[str] = field(default_factory=list)

    def __str__(self) -> str:
        facility_str = ", ".join(self.facilities) if self.facilities else "无"
        return f"{self.name} (区域：{self.zone}, 设施：{facility_str})"


@dataclass
class Section:
    """区段"""
    start_station: Station
    end_station: Station
    line_name: str
    cost: Cost
    duration: Duration

    def __str__(self) -> str:
        return (f"[{self.line_name}] {self.start_station.name} → {self.end_station.name} "
                f"(耗时：{self.duration}, 费用：{self.cost})")


@dataclass
class Journey:
    """旅程"""
    start_station: Station
    end_station: Station
    sections: List[Section] = field(default_factory=list)
    transfers: int = 0

    @property
    def total_cost(self) -> Cost:
        if not self.sections:
            return Cost(0.0)
        total = self.sections[0].cost
        for section in self.sections[1:]:
            total = total + section.cost
        return total

    @property
    def total_duration(self) -> Duration:
        if not self.sections:
            return Duration(0)
        total = self.sections[0].duration
        for section in self.sections[1:]:
            total = total + section.duration
        return total

    def __str__(self) -> str:
        result = [f"旅程：{self.start_station.name} → {self.end_station.name}"]
        result.append(f"  换乘次数：{self.transfers}")
        result.append(f"  总耗时：{self.total_duration}")
        result.append(f"  总费用：{self.total_cost}")
        result.append("  路线详情:")
        for i, section in enumerate(self.sections, 1):
            result.append(f"    {i}. {section}")
        return "\n".join(result)


# ==================== 网络管理类 ====================

class TransitNetwork:
    """公共交通网络"""

    def __init__(self):
        self.stations: Dict[str, Station] = {}
        self.sections: List[Section] = []
        self.lines: Dict[str, List[str]] = {}

    def add_station(self, station: Station) -> None:
        self.stations[station.name] = station

    def add_section(self, section: Section) -> None:
        self.sections.append(section)
        if section.line_name not in self.lines:
            self.lines[section.line_name] = []
        if section.start_station.name not in self.lines[section.line_name]:
            self.lines[section.line_name].append(section.start_station.name)
        if section.end_station.name not in self.lines[section.line_name]:
            self.lines[section.line_name].append(section.end_station.name)

    def get_station(self, name: str) -> Optional[Station]:
        return self.stations.get(name)

    def list_all_stations(self) -> List[Station]:
        return list(self.stations.values())

    def get_network_summary(self) -> Dict[str, Any]:
        return {
            "站点总数": len(self.stations),
            "区段总数": len(self.sections),
            "线路总数": len(self.lines),
            "线路列表": list(self.lines.keys()),
            "区域列表": list(set(s.zone for s in self.stations.values()))
        }

    def find_journey(self, start_name: str, end_name: str,
                     mode: PreferenceMode = PreferenceMode.FASTEST) -> Optional[Journey]:
        start = self.get_station(start_name)
        end = self.get_station(end_name)

        if not start or not end:
            return None

        if start_name == end_name:
            return Journey(start, end)

        adjacency: Dict[str, List[Section]] = {}
        for section in self.sections:
            if section.start_station.name not in adjacency:
                adjacency[section.start_station.name] = []
            adjacency[section.start_station.name].append(section)

        all_paths: List[List[Section]] = []
        queue = [(start_name, [])]
        visited_paths = set()

        while queue and len(all_paths) < 10:
            current, path = queue.pop(0)

            if current == end_name:
                all_paths.append(path)
                continue

            for section in adjacency.get(current, []):
                new_path = path + [section]
                path_key = tuple(s.line_name for s in new_path)
                if path_key not in visited_paths:
                    visited_paths.add(path_key)
                    queue.append((section.end_station.name, new_path))

        if not all_paths:
            return None

        best_path = self._select_best_path(all_paths, mode)

        transfers = 0
        for i in range(1, len(best_path)):
            if best_path[i].line_name != best_path[i-1].line_name:
                transfers += 1

        return Journey(start, end, best_path, transfers)

    def _select_best_path(self, paths: List[List[Section]],
                          mode: PreferenceMode) -> List[Section]:
        if mode == PreferenceMode.FASTEST:
            return min(paths, key=lambda p: sum(s.duration.minutes for s in p))
        elif mode == PreferenceMode.CHEAPEST:
            return min(paths, key=lambda p: sum(s.cost.amount for s in p))
        else:
            def count_transfers(path):
                transfers = 0
                for i in range(1, len(path)):
                    if path[i].line_name != path[i-1].line_name:
                        transfers += 1
                return transfers
            return min(paths, key=count_transfers)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "stations": [
                {"name": s.name, "zone": s.zone, "facilities": s.facilities}
                for s in self.stations.values()
            ],
            "sections": [
                {
                    "start": s.start_station.name,
                    "end": s.end_station.name,
                    "line": s.line_name,
                    "cost": s.cost.amount,
                    "duration": s.duration.minutes
                }
                for s in self.sections
            ]
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'TransitNetwork':
        network = cls()
        stations = {}
        for s_data in data.get("stations", []):
            station = Station(
                name=s_data["name"],
                zone=s_data["zone"],
                facilities=s_data.get("facilities", [])
            )
            stations[station.name] = station
            network.add_station(station)
        for sec_data in data.get("sections", []):
            section = Section(
                start_station=stations[sec_data["start"]],
                end_station=stations[sec_data["end"]],
                line_name=sec_data["line"],
                cost=Cost(sec_data["cost"]),
                duration=Duration(sec_data["duration"])
            )
            network.add_section(section)
        return network

    def save_to_file(self, filepath: str) -> None:
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(self.to_dict(), f, ensure_ascii=False, indent=2)

    @classmethod
    def load_from_file(cls, filepath: str) -> 'TransitNetwork':
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)
        return cls.from_dict(data)


# ==================== 交互式菜单界面 ====================

class TransitAdvisorUI:
    """交互式界面"""

    def __init__(self):
        self.network: Optional[TransitNetwork] = None
        self.running = True

    def show_menu(self) -> None:
        print("\n--- 菜单 ---")
        print("1. 列出所有站点")
        print("2. 查询旅程")
        print("3. 显示网络摘要")
        print("4. 加载网络")
        print("0. 退出")

    def list_stations(self) -> None:
        if not self.network:
            print("⚠ 请先加载网络 (选项 4)")
            return
        stations = self.network.list_all_stations()
        print(f"\n共 {len(stations)} 个站点:")
        for station in stations:
            print(f"  • {station}")

    def query_journey(self) -> None:
        if not self.network:
            print("⚠ 请先加载网络 (选项 4)")
            return

        start = input("起点站：").strip()
        end = input("终点站：").strip()

        print("偏好：1.最快 2.最便宜 3.最少换乘")
        mode_choice = input("选择 (默认 1): ").strip()

        mode_map = {"1": PreferenceMode.FASTEST,
                    "2": PreferenceMode.CHEAPEST,
                    "3": PreferenceMode.LEAST_TRANSFERS}
        mode = mode_map.get(mode_choice, PreferenceMode.FASTEST)

        journey = self.network.find_journey(start, end, mode)

        if journey is None:
            print(f"⚠ 未找到从 '{start}' 到 '{end}' 的旅程")
        elif not journey.sections:
            print(f"✓ 起点终点相同")
        else:
            print(f"\n{journey}")

    def show_summary(self) -> None:
        if not self.network:
            print("⚠ 请先加载网络 (选项 4)")
            return
        summary = self.network.get_network_summary()
        print(f"\n站点：{summary['站点总数']}, 区段：{summary['区段总数']}, 线路：{summary['线路总数']}")
        print(f"线路：{', '.join(summary['线路列表'])}")

    def load_network(self) -> None:
        filepath = input("文件路径：").strip()
        if not filepath:
            print("⚠ 路径不能为空")
            return
        try:
            self.network = TransitNetwork.load_from_file(filepath)
            print("✓ 加载成功")
        except Exception as e:
            print(f"⚠ 加载失败：{e}")

    def run(self) -> None:
        print("=== 智能公共交通顾问 ===")
        while self.running:
            self.show_menu()
            choice = input("选择：").strip()
            if choice == "1":
                self.list_stations()
            elif choice == "2":
                self.query_journey()
            elif choice == "3":
                self.show_summary()
            elif choice == "4":
                self.load_network()
            elif choice == "0":
                print("再见!")
                self.running = False
            else:
                print("⚠ 无效选择")


if __name__ == "__main__":
    TransitAdvisorUI().run()
