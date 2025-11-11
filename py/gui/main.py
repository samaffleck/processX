from imgui_bundle import imgui, immapp, hello_imgui

# Local imports
from menu_bar import show_menus
from default_layout import create_default_layout
from unit_operations import show_unit_operations
from flowsheet import show_flowsheet
from unit_properties import show_unit_properties
from fluid_packages import show_fluid_packages
from log import show_log
from window_state import get_window_dict


def render_window(name: str, state_key: str, render_func):
    """Helper function to render a window with state management."""
    window_dict = get_window_dict()
    
    if not window_dict[state_key]:
        return
    
    is_visible, p_open = imgui.begin(name, window_dict[state_key])
    if p_open is not None:
        window_dict[state_key] = p_open
    if is_visible:
        render_func()
    imgui.end()


def show_gui():
    """Render all dockable windows."""
    windows = [
        ("Unit Operations", "unit_ops_open", show_unit_operations),
        ("Flowsheet", "flowsheet_open", show_flowsheet),
        ("Properties", "properties_open", show_unit_properties),
        ("Fluid Packages", "fluid_packages_open", show_fluid_packages),
        ("Log", "log_open", show_log),
    ]
    
    for name, state_key, render_func in windows:
        render_window(name, state_key, render_func)
    


def main():
    runner_params = hello_imgui.RunnerParams()
    runner_params.app_window_params.window_title = "ProcessX"
    runner_params.app_window_params.window_geometry.window_size_state = (
        hello_imgui.WindowSizeState.maximized
    )
    
    # Set up full screen dock space
    runner_params.imgui_window_params.default_imgui_window_type = (
        hello_imgui.DefaultImGuiWindowType.provide_full_screen_dock_space
    )
    runner_params.imgui_window_params.enable_viewports = True
    
    # Enable menu bar with custom menus
    runner_params.imgui_window_params.show_menu_bar = True
    runner_params.imgui_window_params.show_menu_app = False
    runner_params.imgui_window_params.show_menu_view = False
    runner_params.callbacks.show_menus = lambda: show_menus(runner_params)
    
    # Set up docking layout
    runner_params.docking_params = create_default_layout()
    
    # Set up the GUI callback to manually render windows
    runner_params.callbacks.show_gui = show_gui
    
    hello_imgui.run(runner_params)


if __name__ == "__main__":
    main()
