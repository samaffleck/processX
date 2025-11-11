from imgui_bundle import imgui, immapp, hello_imgui
from window_state import get_window_dict


def show_menus(runner_params: hello_imgui.RunnerParams):
    """Create the menu bar with File, Edit, and View menus."""
    # File menu
    if imgui.begin_menu("File"):
        clicked, _ = imgui.menu_item("New", "Ctrl+N", False)
        if clicked:
            # TODO: Implement new file functionality
            pass
        
        clicked, _ = imgui.menu_item("Open", "Ctrl+O", False)
        if clicked:
            # TODO: Implement open file functionality
            pass
        
        clicked, _ = imgui.menu_item("Save", "Ctrl+S", False)
        if clicked:
            # TODO: Implement save file functionality
            pass
        
        imgui.separator()
        
        clicked, _ = imgui.menu_item("Exit", "Ctrl+Q", False)
        if clicked:
            runner_params.app_shall_exit = True
        
        imgui.end_menu()
    
    # Edit menu
    if imgui.begin_menu("Edit"):
        clicked, _ = imgui.menu_item("Undo", "Ctrl+Z", False)
        if clicked:
            # TODO: Implement undo functionality
            pass
        
        clicked, _ = imgui.menu_item("Redo", "Ctrl+Y", False)
        if clicked:
            # TODO: Implement redo functionality
            pass
        
        imgui.separator()
        
        clicked, _ = imgui.menu_item("Cut", "Ctrl+X", False)
        if clicked:
            # TODO: Implement cut functionality
            pass
        
        clicked, _ = imgui.menu_item("Copy", "Ctrl+C", False)
        if clicked:
            # TODO: Implement copy functionality
            pass
        
        clicked, _ = imgui.menu_item("Paste", "Ctrl+V", False)
        if clicked:
            # TODO: Implement paste functionality
            pass
        
        imgui.end_menu()
    
    # View menu
    if imgui.begin_menu("View"):
        # Window visibility toggles
        window_dict = get_window_dict()
        
        clicked, _ = imgui.menu_item("Unit Operations", "", window_dict['unit_ops_open'])
        if clicked:
            window_dict['unit_ops_open'] = not window_dict['unit_ops_open']
        
        clicked, _ = imgui.menu_item("Flowsheet", "", window_dict['flowsheet_open'])
        if clicked:
            window_dict['flowsheet_open'] = not window_dict['flowsheet_open']
        
        clicked, _ = imgui.menu_item("Properties", "", window_dict['properties_open'])
        if clicked:
            window_dict['properties_open'] = not window_dict['properties_open']
        
        clicked, _ = imgui.menu_item("Fluid Packages", "", window_dict['fluid_packages_open'])
        if clicked:
            window_dict['fluid_packages_open'] = not window_dict['fluid_packages_open']
        
        clicked, _ = imgui.menu_item("Log", "", window_dict['log_open'])
        if clicked:
            window_dict['log_open'] = not window_dict['log_open']
        
        imgui.separator()
        
        # Theme selector submenu
        if imgui.begin_menu("Theme"):
            # Get current theme
            tweaked_theme = runner_params.imgui_window_params.tweaked_theme
            current_theme = tweaked_theme.theme
            
            # List of available themes
            # Note: Theme enum names use lowercase with underscores in Python
            themes = [
                ("ImGui Colors Classic", hello_imgui.ImGuiTheme_.imgui_colors_classic),
                ("ImGui Colors Dark", hello_imgui.ImGuiTheme_.imgui_colors_dark),
                ("ImGui Colors Light", hello_imgui.ImGuiTheme_.imgui_colors_light),
                ("Material Flat", hello_imgui.ImGuiTheme_.material_flat),
                ("Photoshop Style", hello_imgui.ImGuiTheme_.photoshop_style),
                ("Gray Variations", hello_imgui.ImGuiTheme_.gray_variations),
                ("Gray Variations Darker", hello_imgui.ImGuiTheme_.gray_variations_darker),
                ("Microsoft Style", hello_imgui.ImGuiTheme_.microsoft_style),
                ("Cherry", hello_imgui.ImGuiTheme_.cherry),
                ("Darcula", hello_imgui.ImGuiTheme_.darcula),
                ("Darcula Darker", hello_imgui.ImGuiTheme_.darcula_darker),
                ("Light Rounded", hello_imgui.ImGuiTheme_.light_rounded),
                ("So Dark Accent Blue", hello_imgui.ImGuiTheme_.so_dark_accent_blue),
                ("So Dark Accent Yellow", hello_imgui.ImGuiTheme_.so_dark_accent_yellow),
                ("So Dark Accent Red", hello_imgui.ImGuiTheme_.so_dark_accent_red),
                ("Black Is Black", hello_imgui.ImGuiTheme_.black_is_black),
                ("White Is White", hello_imgui.ImGuiTheme_.white_is_white),
            ]
            
            for theme_name, theme_enum in themes:
                is_selected = (current_theme == theme_enum)
                clicked, _ = imgui.menu_item(theme_name, "", is_selected)
                if clicked:
                    tweaked_theme.theme = theme_enum
                    hello_imgui.apply_tweaked_theme(tweaked_theme)
            
            imgui.end_menu()
        
        imgui.end_menu()
