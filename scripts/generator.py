#!/usr/bin/env python3
"""
Russia IP Ranges Generator + Hikvision/SmartPSS/Camera Ports
Все диапазоны России + целевые диапазоны для портов 80, 3000, 8000, 37777
"""

import ipaddress
import requests
import argparse
import os
from datetime import datetime

class RussiaIPGenerator:
    def __init__(self):
        # Горячие точки для Hikvision (порт 8000)
        self.hikvision_hotspots = [
            # Ростелеком - массовые установки Hikvision
            "31.173.0.0/16", "95.165.0.0/16", "178.140.0.0/16", "188.170.0.0/16",
            "62.109.0.0/16", "85.26.0.0/16", "79.141.0.0/16", "31.130.0.0/16",
            
            # МТС - бизнес-клиенты
            "37.9.0.0/16", "37.44.0.0/16", "46.180.0.0/16", "46.188.0.0/16",
            "37.110.0.0/16", "37.145.0.0/16",
            
            # Билайн
            "77.37.0.0/16", "78.36.0.0/16", "87.117.0.0/16", "89.22.0.0/16",
            "77.121.0.0/16", "78.107.0.0/16",
            
            # Дом.ru
            "109.111.0.0/16", "176.196.0.0/16", "176.197.0.0/16", "109.124.0.0/16",
            
            # TTK
            "85.26.0.0/16", "79.141.0.0/16", "93.170.0.0/16", "94.139.0.0/16",
            
            # Государственные/бизнес сети
            "85.113.0.0/16", "86.57.0.0/16", "95.24.0.0/16", "128.204.0.0/16",
            "176.108.0.0/16", "178.219.0.0/16", "188.186.0.0/16",
        ]
        
        # SmartPSS диапазоны (порт 3000)
        self.smartpss_hotspots = [
            # Крупные бизнес-сети
            "93.170.0.0/16", "94.139.0.0/16", "95.24.0.0/16",
            "109.124.0.0/16", "109.200.0.0/16", "128.204.0.0/16",
            "176.108.0.0/16", "178.219.0.0/16",
            
            # Сети с офисными камерами
            "85.113.0.0/16", "86.57.0.0/16", "87.117.0.0/16",
            "91.76.0.0/16", "91.224.0.0/16", "92.63.0.0/16"
        ]
        
        # Порт 80 горячие точки (веб-интерфейсы)
        self.web_hotspots = [
            # Провайдеры с массовыми веб-интерфейсами
            "31.173.0.0/16", "95.165.0.0/16", "178.140.0.0/16",
            "37.9.0.0/16", "46.180.0.0/16", "77.37.0.0/16",
            "109.111.0.0/16", "176.196.0.0/16", "85.26.0.0/16"
        ]
        
        # Порт 37777 горячие точки (камеры Dahua/Hikvision)
        self.camera_37777_hotspots = [
            # Сети с большим количеством IP-камер
            "31.173.0.0/16", "95.165.0.0/16", "178.140.0.0/16",  # Ростелеком
            "37.9.0.0/16", "46.180.0.0/16",                      # МТС
            "77.37.0.0/16", "78.36.0.0/16",                      # Билайн
            "109.111.0.0/16", "176.196.0.0/16",                  # Дом.ru
            "85.26.0.0/16", "79.141.0.0/16",                     # TTK
            
            # Бизнес-сети (офисные камеры)
            "93.170.0.0/16", "94.139.0.0/16", "95.24.0.0/16",
            "109.124.0.0/16", "128.204.0.0/16",
            
            # Региональные провайдеры
            "31.41.0.0/16", "46.138.0.0/16", "83.239.0.0/16",
            "89.189.0.0/16", "91.76.0.0/16", "92.63.0.0/16"
        ]

    def download_all_russia_ranges(self):
        """Скачивает ВСЕ диапазоны России"""
        sources = [
            "http://www.ipdeny.com/ipblocks/data/countries/ru.zone",
            "https://raw.githubusercontent.com/herrbischoff/country-ip-blocks/master/ipv4/ru.cidr"
        ]
        
        all_cidr = []
        
        for url in sources:
            try:
                print(f"📡 Загрузка с {url.split('/')[-1]}...")
                response = requests.get(url, timeout=15)
                if response.status_code == 200:
                    cidr_list = [line.strip() for line in response.text.splitlines() if line.strip() and not line.startswith('#')]
                    all_cidr.extend(cidr_list)
                    print(f"✅ Загружено {len(cidr_list)} CIDR блоков")
            except Exception as e:
                print(f"❌ Ошибка: {e}")
                continue
        
        # Убираем дубликаты
        unique_cidr = list(set(all_cidr))
        print(f"📊 Всего уникальных CIDR блоков России: {len(unique_cidr)}")
        return unique_cidr

    def cidr_to_range(self, cidr):
        """Конвертирует CIDR в диапазон IP"""
        try:
            network = ipaddress.ip_network(cidr, strict=False)
            return f"{network.network_address}-{network.broadcast_address}"
        except Exception as e:
            return None

    def generate_specialized_ranges(self, all_russia_ranges):
        """Генерирует специализированные диапазоны для портов"""
        
        # Все диапазоны России
        all_ranges = []
        for cidr in all_russia_ranges:
            ip_range = self.cidr_to_range(cidr)
            if ip_range:
                all_ranges.append(ip_range)
        
        # Hikvision диапазоны (порт 8000)
        hikvision_ranges = []
        for cidr in self.hikvision_hotspots:
            ip_range = self.cidr_to_range(cidr)
            if ip_range and ip_range not in hikvision_ranges:
                hikvision_ranges.append(ip_range)
        
        # SmartPSS диапазоны (порт 3000)
        smartpss_ranges = []
        for cidr in self.smartpss_hotspots:
            ip_range = self.cidr_to_range(cidr)
            if ip_range and ip_range not in smartpss_ranges:
                smartpss_ranges.append(ip_range)
        
        # Порт 80 диапазоны
        web_ranges = []
        for cidr in self.web_hotspots:
            ip_range = self.cidr_to_range(cidr)
            if ip_range and ip_range not in web_ranges:
                web_ranges.append(ip_range)
        
        # Порт 37777 диапазоны (камеры)
        camera_37777_ranges = []
        for cidr in self.camera_37777_hotspots:
            ip_range = self.cidr_to_range(cidr)
            if ip_range and ip_range not in camera_37777_ranges:
                camera_37777_ranges.append(ip_range)
        
        return {
            "all_russia": all_ranges,
            "hikvision_8000": hikvision_ranges,
            "smartpss_3000": smartpss_ranges,
            "web_80": web_ranges,
            "camera_37777": camera_37777_ranges
        }

    def save_ranges(self, ranges_dict, output_dir="russia_ranges"):
        """Сохраняет все диапазоны в отдельные файлы"""
        
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)
        
        saved_files = []
        
        for range_type, ranges in ranges_dict.items():
            filename = os.path.join(output_dir, f"{range_type}.txt")
            
            with open(filename, 'w', encoding='utf-8') as f:
                f.write(f"# Russia IP Ranges - {range_type.upper()}\n")
                f.write(f"# Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f"# Total ranges: {len(ranges)}\n\n")
                
                for ip_range in ranges:
                    f.write(f"{ip_range}\n")
            
            print(f"✅ Создан: {filename} ({len(ranges):,} диапазонов)")
            saved_files.append(filename)
        
        return saved_files

    def create_combined_file(self, ranges_dict, output_file="all_russia_combined.txt"):
        """Создает комбинированный файл со всеми диапазонами"""
        
        all_combined = []
        for ranges in ranges_dict.values():
            all_combined.extend(ranges)
        
        # Убираем дубликаты
        unique_ranges = list(set(all_combined))
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(f"# ALL RUSSIA IP RANGES + HIKVISION + SMARTPSS + CAMERAS\n")
            f.write(f"# Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"# Includes: All Russia + Hikvision(8000) + SmartPSS(3000) + Web(80) + Cameras(37777)\n")
            f.write(f"# Total unique ranges: {len(unique_ranges)}\n\n")
            
            for ip_range in unique_ranges:
                f.write(f"{ip_range}\n")
        
        print(f"🎯 Комбинированный файл: {output_file} ({len(unique_ranges):,} диапазонов)")
        return output_file

    def generate_scan_configs(self, ranges_dict):
        """Генерирует конфиги для сканирования"""
        
        configs_dir = "scan_configs"
        if not os.path.exists(configs_dir):
            os.makedirs(configs_dir)
        
        # Конфиг для Masscan
        masscan_config = f"""# Masscan config for Russia IP ranges
# Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

rate = 10000
ports = 80,3000,8000,37777,443,554,8080,7200
range = all_russia_combined.txt
output-format = xml
output-filename = russia_scan.xml
banners = true
http-user-agent = Mozilla/5.0
"""
        
        with open(os.path.join(configs_dir, "masscan_russia.conf"), 'w') as f:
            f.write(masscan_config)
        
        # Конфиг для Nmap
        nmap_script = f"""# Nmap scan for Russia targets
# Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

# Quick port scan (все основные порты)
nmap -iL all_russia_combined.txt -p 80,3000,8000,37777 --open -T4 -oA russia_quick

# Hikvision specific
nmap -iL hikvision_8000.txt -p 8000,80,443,554 --script http-title -T4 -oA hikvision_scan

# SmartPSS specific  
nmap -iL smartpss_3000.txt -p 3000,7200,7201 --script banner -T4 -oA smartpss_scan

# Web interfaces
nmap -iL web_80.txt -p 80,443,8080 --script http-title,http-headers -T4 -oA web_scan

# Camera port 37777 (Dahua/Hikvision)
nmap -iL camera_37777.txt -p 37777 --script banner -sV -T4 -oA camera_37777_scan

# Comprehensive camera scan
nmap -iL all_russia_combined.txt -p 80,443,554,8000,8080,37777,3000 --script http-title,rtsp-methods -T4 -oA comprehensive_camera_scan
"""
        
        with open(os.path.join(configs_dir, "nmap_commands.txt"), 'w') as f:
            f.write(nmap_script)

        # Конфиг для NESCA
        nesa_ports = "80,3000,8000,37777,443,554,8080,7200,8443"
        nesa_config = f"""# NESCA config for Russia camera scan
# Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

# Основные порты для сканирования
PORTS = {nesa_ports}

# Файл с целями
TARGET_FILE = all_russia_combined.txt

# Рекомендуемые настройки:
# - Используйте модуль HTTP для портов 80,443,8000,8080
# - Используйте модуль RTSP для порта 554
# - Для порта 37777 используйте banner grabbing
# - Для порта 3000 проверяйте SmartPSS

# Команда для NESCA:
# nesa --file all_russia_combined.txt --ports {nesa_ports} --http --rtsp
"""
        
        with open(os.path.join(configs_dir, "nesca_config.txt"), 'w') as f:
            f.write(nesa_config)
        
        print(f"🔧 Конфиги созданы в папке: {configs_dir}")
        return configs_dir

def main():
    banner = """
    🌐 Russia IP Ranges Generator + Camera Ports
    📊 ВСЕ диапазоны России + целевые диапазоны для портов
    🎯 Порты: 80 (веб), 3000 (SmartPSS), 8000 (Hikvision), 37777 (камеры)
    """
    print(banner)
    
    parser = argparse.ArgumentParser(description='Генератор всех диапазонов России + целевые диапазоны')
    parser.add_argument('-o', '--output', default='all_russia_combined.txt',
                       help='Комбинированный файл (по умолчанию: all_russia_combined.txt)')
    parser.add_argument('-d', '--directory', default='russia_ranges',
                       help='Папка для отдельных файлов (по умолчанию: russia_ranges)')
    parser.add_argument('-c', '--configs', action='store_true',
                       help='Сгенерировать конфиги для сканирования')
    
    args = parser.parse_args()
    
    generator = RussiaIPGenerator()
    
    print("🔄 Загрузка ВСЕХ диапазонов России...")
    all_russia_cidr = generator.download_all_russia_ranges()
    
    if not all_russia_cidr:
        print("❌ Не удалось загрузить диапазоны России")
        return
    
    print("🔄 Генерация специализированных диапазонов...")
    ranges_dict = generator.generate_specialized_ranges(all_russia_cidr)
    
    print("💾 Сохранение файлов...")
    
    # Сохраняем отдельные файлы
    saved_files = generator.save_ranges(ranges_dict, args.directory)
    
    # Создаем комбинированный файл
    combined_file = generator.create_combined_file(ranges_dict, args.output)
    
    # Генерируем конфиги если нужно
    if args.configs:
        generator.generate_scan_configs(ranges_dict)
    
    # Статистика
    print(f"\n📊 СТАТИСТИКА:")
    print(f"🌐 Всего Россия: {len(ranges_dict['all_russia']):,} диапазонов")
    print(f"🎯 Hikvision (8000): {len(ranges_dict['hikvision_8000']):,} диапазонов") 
    print(f"🔧 SmartPSS (3000): {len(ranges_dict['smartpss_3000']):,} диапазонов")
    print(f"🌍 Web (80): {len(ranges_dict['web_80']):,} диапазонов")
    print(f"📹 Cameras (37777): {len(ranges_dict['camera_37777']):,} диапазонов")
    print(f"🚀 Комбинированный: {combined_file}")
    
    print(f"\n🎯 ИСПОЛЬЗОВАНИЕ:")
    print(f"  • Все диапазоны: {args.output}")
    print(f"  • Hikvision (8000): {args.directory}/hikvision_8000.txt")
    print(f"  • SmartPSS (3000): {args.directory}/smartpss_3000.txt") 
    print(f"  • Web (80): {args.directory}/web_80.txt")
    print(f"  • Cameras (37777): {args.directory}/camera_37777.txt")
    print(f"\n⚡ Для NESCA используйте: {args.output}")
    print(f"🎯 Ключевые порты: 80, 3000, 8000, 37777")

if __name__ == "__main__":
    main()