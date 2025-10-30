# Nesca v2r GUI - User Guide

## 📖 Quick Start Guide

This guide will help new users understand the Nesca v2r GUI interface and all its features.

## 🎯 Main Window Overview

The GUI is divided into two main tabs:

### 1. Configuration Tab
Where you set up your scan parameters.

### 2. Results Tab
Where scan results are displayed.

---

## ⚙️ Configuration Tab - Detailed Guide

### Welcome Banner
At the top, you'll see a helpful banner that says:
> 💡 **New to scanning?** Hover over any field for detailed help

This banner reminds you that tooltips are available for every field.

### Scan Mode

**What it does:** Choose how you want to specify your targets.

**Options:**
- **IP Range**: Scan a range of IP addresses
  - Example: `192.168.1.0/24`
  - Best for: Local network scanning
  
- **DNS Scan**: Scan a domain name
  - Example: `example.com`
  - Best for: Web server scanning
  
- **Import from File**: Load IPs from a text file
  - Format: One IP per line
  - Best for: Bulk scanning

**Tooltip help:** Hover to see what each mode does and when to use it.

### Target Field

**What it does:** Enter your scan target.

**Example formats:**

**CIDR notation** (most common):
- `192.168.1.0/24` → Scans 192.168.1.1-254
- `10.0.0.0/16` → Scans 10.0.x.x (65,536 hosts)

**Range notation:**
- `192.168.1.1-192.168.1.100` → Scans first 100 hosts
- `192.168.1.1-254` → All hosts in subnet

**Single IP:**
- `192.168.1.1` → Just one host

**DNS:**
- `example.com` → Domain name

**File:**
- `/path/to/ips.txt` → Import from file

**Tooltip help:** Hover to see all formatting examples and explanations.

### Ports Field

**What it does:** Specify which ports to scan.

**Common port categories:**

**Web Servers:**
- `80` - HTTP
- `443` - HTTPS
- `8080` - HTTP Alternative
- `8000` - HTTP Alternative
- `8443` - HTTPS Alternative

**Media/Streaming:**
- `554` - RTSP (cameras)
- `1935` - RTMP
- `9000` - Streaming

**Cameras:**
- `37777` - RVI/Hikvision
- `34567` - Some cameras
- `8000` - Some cameras
- `88` - Some cameras

**Other Services:**
- `21` - FTP
- `22` - SSH
- `23` - Telnet
- `3389` - RDP
- `3306` - MySQL

**Format:** Separate with commas
- Example: `80,443,8080`
- Or use ranges: `80-100` or `8000-8100`

**Default:** Pre-filled with common web and camera ports

**Tooltip help:** Hover to see categorized port list.

### Threads

**What it does:** Controls how many parallel connections happen at once.

**Understanding threads:**
- More threads = faster scanning
- But also uses more CPU and network

**Recommended values:**
- **50-100**: Safe, normal usage (100 is default)
- **100-200**: Faster, for stable networks
- **200+**: Very fast, only for very stable networks

**Tooltip help:** Hover to see detailed recommendations based on your network.

### ⚙️ Advanced Options

Expand the Advanced Options group to see more settings.

#### Timeout

**What it does:** How long to wait for a response before giving up.

**Understanding timeouts:**
- Too low = missed hosts (false negatives)
- Too high = slow scanning

**Recommended:**
- **1000-3000ms**: Fast networks (default: 3000ms)
- **5000ms**: Slower/remote networks
- **10000ms+**: Very slow or unreliable networks

**Tooltip help:** Hover to understand how to choose the right timeout.

#### Verify SSL Certificates

**What it does:** Check if SSL certificates are valid.

**Options:**
- **Checked**: Only accept valid certificates (more secure)
- **Unchecked**: Accept any certificate, even self-signed

**When to uncheck:**
- Scanning IoT devices
- Scanning cameras
- Scanning devices with self-signed certificates
- Testing environments

**Tooltip help:** Hover to see when to use each option.

#### Adaptive Scan

**What it does:** Automatically adjusts thread count based on network performance.

**How it works:**
1. Starts with your set thread count
2. Monitors network latency and errors
3. Increases threads if network is stable
4. Decreases if too many timeouts occur

**Best for:** Large scans on varying network conditions

**Tooltip help:** Hover to see detailed explanation of the algorithm.

#### Smart Scan

**What it does:** Prioritizes common ports for faster results.

**How it works:**
1. First scans most likely ports (80, 443, etc.)
2. After finding open ports, focuses on their protocols
3. Skips unlikely combinations

**Effect:**
- Faster results
- Fewer false positives

**Best for:** Quick reconnaissance

**Tooltip help:** Hover to understand the optimization strategy.

### 📊 Export Options

#### Export Format

**What it does:** Choose how results are saved.

**Options:**
- **JSON**: Structured data format, easy for scripts
  - Best for: Developers, automation
  
- **CSV**: Spreadsheet format
  - Best for: Excel, Google Sheets, general users
  
- **Both**: Save in both formats
  - Best for: Maximum flexibility

**Tooltip help:** Hover to see recommendations.

---

## 📋 Results Tab - Detailed Guide

### Results Info Banner

At the top of the results table:
> 📋 **Results Table:** Double-click any row to view detailed information | **Export:** Save results to JSON/CSV | **Clear:** Remove all results

This banner explains the main actions.

### Results Table

**Columns:**
- **IP**: The target IP address
- **Port**: The scanned port
- **Type**: Device/service type found
- **Timestamp**: When it was found
- **Data**: Additional information

**Interactions:**
- **Double-click any row**: View detailed information
- **Single-click**: Select row
- **Column headers**: Sort results

### Details Panel

**Right side of results tab:**
- Shows detailed information for selected row
- Includes login credentials if found
- Shows additional metadata

**Interactions:**
- Automatically updates when you double-click a row
- Expandable tree structure

### Buttons

**Export Results:**
- Saves all results to file
- Format based on your export settings
- Opens file dialog to choose location

**Clear Results:**
- Removes all results from table
- Resets statistics
- Useful before starting new scan

### Statistics

**Bottom of results tab:**
Shows counts for different device types:
- Total found
- Cameras
- Authentications
- FTP servers
- SSH servers
- Other devices

---

## 🚀 Quick Start Example

### Your First Scan

1. **Leave defaults** (good for most scans)
   - Scan Mode: IP Range
   - Ports: Default ports loaded
   - Threads: 100

2. **Enter your target:**
   - Example: `192.168.1.0/24`
   - Or: `example.com`

3. **Click "▶ Start Scan"**

4. **Watch results appear** in the Results tab

5. **Double-click any result** to see details

6. **Export** when done

### Common Scenarios

**Scan Local Network:**
- Target: `192.168.1.0/24`
- Ports: Default (already loaded)
- Threads: 100
- Start scan

**Scan for Cameras:**
- Target: `192.168.1.0/24`
- Ports: `80,443,554,8000,8080,37777,34567,9000`
- Uncheck "Verify SSL"
- Start scan

**Quick Reconnaissance:**
- Target: `10.0.0.0/16`
- Ports: `80,443,22,21,3389`
- Check "Smart scan"
- Threads: 200
- Start scan

**Slow/Remote Network:**
- Target: Your target
- Timeout: 5000 ms
- Check "Adaptive scan"
- Threads: 50
- Start scan

---

## 💡 Pro Tips

1. **Use tooltips:** Hover over everything for detailed explanations

2. **Start small:** Test with `/24` subnets before large scans

3. **Check results table:** Double-click to see if credentials were found

4. **Adjust threads:** If you see many timeouts, reduce threads

5. **Use adaptive mode:** For large scans on unpredictable networks

6. **Smart scan:** Use for quick checks, disable for comprehensive scans

7. **Export regularly:** Save results as you go

8. **Clear between scans:** Start fresh for each target

---

## 🆘 Troubleshooting

**No results found:**
- Check if target is reachable
- Verify port numbers
- Increase timeout
- Reduce threads

**Many timeouts:**
- Increase timeout value
- Reduce thread count
- Enable adaptive scan

**Slow scanning:**
- Increase threads (if network is stable)
- Use smart scan mode
- Reduce number of ports

**SSL errors:**
- Uncheck "Verify SSL certificates"
- Common for IoT devices

**Export not working:**
- Check disk space
- Verify write permissions
- Try different export format

---

## 📚 Additional Resources

- See tooltips in the GUI for field-specific help
- Check main README for command-line options
- Review CHANGELOG for latest features

---

## 🎉 You're Ready!

The GUI provides comprehensive help at every step. Remember:

- **Hover** over fields for detailed tooltips
- **Read** the helpful banners
- **Start** with defaults
- **Experiment** with settings
- **Double-click** results for details

Happy scanning! 🔍

