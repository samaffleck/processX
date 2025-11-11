from imgui_bundle import imgui, immapp, hello_imgui
from typing import List


def create_default_docking_splits() -> List[hello_imgui.DockingSplit]:
    """
    Create the default docking splits for ProcessX layout:
    
    Layout structure:
        ┌──────────┬──────────────────┬──────────────┐
        │          │   Flowsheet      │  Properties  │
        │   Log    │   (top right)     │  (top right) │
        │  (25%)   │                  │              │
        │          ├──────────────────┴──────────────┤
        │          │                                  │
        │          │      Palette (bottom right)      │
        │          │                                  │
        └──────────┴──────────────────────────────────┘
    """
    # Step 1: Split MainDockSpace left to create Log area (25% width)
    # This leaves MainDockSpace as the right side (75% width)
    split_left = hello_imgui.DockingSplit()
    split_left.initial_dock = "MainDockSpace"
    split_left.new_dock = "LogSpace"
    split_left.direction = imgui.Dir.left
    split_left.ratio = 0.25
    
    # Step 2: Split MainDockSpace down to create bottom area for Palette (25% height)
    # This leaves MainDockSpace as the top right area (75% height)
    split_bottom = hello_imgui.DockingSplit()
    split_bottom.initial_dock = "MainDockSpace"
    split_bottom.new_dock = "BottomRightSpace"
    split_bottom.direction = imgui.Dir.down
    split_bottom.ratio = 0.25
    
    # Step 3: Split MainDockSpace (top right) right to create Properties area (50% width of top right)
    # This leaves MainDockSpace as Flowsheet (50% width of top right)
    split_right = hello_imgui.DockingSplit()
    split_right.initial_dock = "MainDockSpace"
    split_right.new_dock = "RightSpace"
    split_right.direction = imgui.Dir.right
    split_right.ratio = 0.50
    
    splits = [split_left, split_bottom, split_right]
    return splits


def create_dockable_windows() -> List[hello_imgui.DockableWindow]:
    """
    Create the dockable windows for ProcessX.
    
    Note: All windows have empty gui_function since we render them manually in show_gui().
    The DockableWindow entries are just for docking layout - they tell hello_imgui where to dock windows.
    """
    windows = []
    
    # Log - left side (25% width)
    log_window = hello_imgui.DockableWindow()
    log_window.label = "Log"
    log_window.dock_space_name = "LogSpace"
    log_window.gui_function = lambda: None  # Empty function - we render manually
    log_window.call_begin_end = False
    log_window.is_visible = True
    windows.append(log_window)
    
    # Flowsheet - top right, shares space with Properties (50% of top right)
    flowsheet_window = hello_imgui.DockableWindow()
    flowsheet_window.label = "Flowsheet"
    flowsheet_window.dock_space_name = "MainDockSpace"
    flowsheet_window.gui_function = lambda: None  # Empty function - we render manually
    flowsheet_window.call_begin_end = False
    flowsheet_window.is_visible = True
    windows.append(flowsheet_window)
    
    # Properties - top right, shares space with Flowsheet (50% of top right)
    properties_window = hello_imgui.DockableWindow()
    properties_window.label = "Properties"
    properties_window.dock_space_name = "RightSpace"
    properties_window.gui_function = lambda: None  # Empty function - we render manually
    properties_window.call_begin_end = False
    properties_window.is_visible = True
    windows.append(properties_window)
    
    # Palette (Unit Operations) - bottom right
    palette_window = hello_imgui.DockableWindow()
    palette_window.label = "Unit Operations"
    palette_window.dock_space_name = "BottomRightSpace"
    palette_window.gui_function = lambda: None  # Empty function - we render manually
    palette_window.call_begin_end = False
    palette_window.is_visible = True
    windows.append(palette_window)
    
    # Fluid Packages - can dock anywhere
    fluid_packages_window = hello_imgui.DockableWindow()
    fluid_packages_window.label = "Fluid Packages"
    fluid_packages_window.dock_space_name = "RightSpace"
    fluid_packages_window.gui_function = lambda: None  # Empty function - we render manually
    fluid_packages_window.call_begin_end = False
    fluid_packages_window.is_visible = True
    windows.append(fluid_packages_window)
    
    return windows


def create_default_layout() -> hello_imgui.DockingParams:
    """Create the default docking layout for ProcessX."""
    docking_params = hello_imgui.DockingParams()
    docking_params.docking_splits = create_default_docking_splits()
    docking_params.dockable_windows = []
    return docking_params
