# GUI File Browse Feature

## 🎯 Overview

Added a user-friendly file browser dialog for the "Import from File" mode in the Nesca v2r GUI.

## ✨ New Feature

### File Browse Button

A "Browse..." button has been added next to the Target field that appears when "Import from File" mode is selected.

**How it works:**
1. User selects "Import from File" from the Scan Mode dropdown
2. Browse button appears automatically
3. Click "Browse..." to open native file dialog
4. Select a `.txt` file containing IP addresses
5. File path is automatically filled in the Target field

## 🔧 Implementation

### UI Changes

**Before:**
```
Target: [________________________]
```

**After:**
```
Target: [________________________] [Browse...]
```

### Component Updates

**Header (`src/ui/ScanConfiguration.h`):**
- Added `QPushButton *m_browseButton` member
- Added `onBrowseFile()` slot
- Added `updateFileBrowseVisibility()` helper

**Source (`src/ui/ScanConfiguration.cpp`):**
- Added `#include <QFileDialog>`
- Created horizontal layout for Target + Browse button
- Implemented file dialog with filter for .txt files
- Automatic visibility toggle based on scan mode

## 📋 File Dialog Details

**Dialog Title:** "Select IP Address File"

**Filters:**
- Text Files (*.txt)
- All Files (*)

**Default Location:** User's current directory

## 🎨 User Experience

### Visibility Behavior

The Browse button is **only visible** when:
- Scan Mode is set to "Import from File"

The Browse button is **hidden** when:
- Scan Mode is set to "IP Range"
- Scan Mode is set to "DNS Scan"

### Visual Design

- **Styling:** Matches modern button design
- **Placement:** Next to Target field on the right
- **Width:** Minimum 80px
- **Padding:** 8px x 16px
- **Tooltip:** "Browse for a file to import IP addresses from"

## 🔄 Workflow

1. User selects "Import from File"
2. Browse button appears
3. User clicks "Browse..."
4. Native file dialog opens
5. User selects a .txt file
6. Path is inserted into Target field
7. User can still manually edit the path
8. Scan proceeds normally

## 💡 Benefits

1. **User-Friendly:** No need to manually type file paths
2. **Platform Native:** Uses Qt's native file dialogs
3. **Error Prevention:** Visual file picker reduces typos
4. **Consistent UX:** Standard file selection pattern
5. **Flexible:** Users can still type paths manually if needed

## 📝 File Format

The imported file should contain one IP address per line:

```
192.168.1.1
192.168.1.2
192.168.1.3
...
```

## 🔍 Code Example

```cpp
void ScanConfiguration::onBrowseFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select IP Address File"),
        QString(),
        tr("Text Files (*.txt);;All Files (*)")
    );
    
    if (!fileName.isEmpty()) {
        m_targetEdit->setText(fileName);
    }
}
```

## ✅ Testing

- ✅ Browse button appears in import mode
- ✅ Browse button hidden in IP/DNS modes
- ✅ File dialog opens correctly
- ✅ Selected file path fills Target field
- ✅ Manual input still works
- ✅ Proper file filters applied
- ✅ No compilation errors
- ✅ No runtime errors

## 🎉 Result

The GUI now provides a professional, user-friendly way to select files for import mode, making it easier for users to work with IP address lists without manual path entry.

---

**Try it out:**
1. Launch `./nesca-gui`
2. Select "Import from File" from Scan Mode
3. Click the "Browse..." button
4. Select your IP list file!

