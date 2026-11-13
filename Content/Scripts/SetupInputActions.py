"""
SetupInputActions.py
Script de automatización para Unreal Engine 5.6.
Crea los Input Actions (IA_Reload, IA_Ability, IA_Interact) en Content/Variant_Shooter/Input/Actions/
y los asigna automáticamente al Input Mapping Context (IMC_Weapons).

Cómo ejecutarlo en Unreal Editor:
1. Abre la ventana 'Output Log' en Unreal Editor.
2. En la barra inferior del log, cambia el selector de 'Cmd' a 'Python'.
3. Pega o ejecuta:
   py "Z:/Unreal Engine/projects/Test/TFPEV2/Content/Scripts/SetupInputActions.py"
"""

import unreal

def setup_enhanced_input():
    actions_path = "/Game/Variant_Shooter/Input/Actions"
    imc_path = "/Game/Variant_Shooter/Input/IMC_Weapons"

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    # 1. Definir las acciones a crear (Nombre, Tecla por defecto)
    input_actions = [
        ("IA_Reload", "R"),
        ("IA_Ability", "Q"),
        ("IA_Interact", "E")
    ]

    created_actions = {}

    for action_name, default_key in input_actions:
        full_path = f"{actions_path}/{action_name}"

        # Verificar si el asset ya existe
        if editor_asset_lib.does_asset_exist(full_path):
            unreal.log(f"[PEV2 Input] Asset ya existente: {full_path}")
            act = editor_asset_lib.load_asset(full_path)
        else:
            # Crear el Input Action
            act = asset_tools.create_asset(
                asset_name=action_name,
                package_path=actions_path,
                asset_class=unreal.InputAction,
                factory=None
            )
            if act:
                # ValueType = Boolean (Digital)
                act.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
                editor_asset_lib.save_asset(full_path)
                unreal.log(f"[PEV2 Input] Creado exitosamente: {full_path}")
            else:
                unreal.log_warning(f"[PEV2 Input] No se pudo crear: {full_path}")

        if act:
            created_actions[action_name] = (act, default_key)

    # 2. Agregar los mappings a IMC_Weapons
    if editor_asset_lib.does_asset_exist(imc_path):
        imc = editor_asset_lib.load_asset(imc_path)
        if imc and isinstance(imc, unreal.InputMappingContext):
            existing_actions = [mapping.get_editor_property("action") for mapping in imc.get_editor_property("mappings")]

            for action_name, (act_asset, key_str) in created_actions.items():
                if act_asset not in existing_actions:
                    key = unreal.Key(key_name=key_str)
                    mapping = imc.add_mapping(act_asset, key)
                    unreal.log(f"[PEV2 Input] Mapeada acción {action_name} a la tecla '{key_str}' en {imc_path}")
                else:
                    unreal.log(f"[PEV2 Input] {action_name} ya estaba mapeada en {imc_path}")

            editor_asset_lib.save_asset(imc_path)
            unreal.log("[PEV2 Input] IMC_Weapons guardado exitosamente.")
    else:
        unreal.log_warning(f"[PEV2 Input] No se encontró el Input Mapping Context en: {imc_path}")

if __name__ == "__main__":
    setup_enhanced_input()
