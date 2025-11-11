import bpy
import mathutils
from pathlib import Path

current_camera = bpy.context.object
assert(current_camera is not None)
assert(current_camera.name.startswith("CameraRigs/") and current_camera.name.count('/') == 2)
current_rig = current_camera.parent

current_rig.matrix_world = current_camera.matrix_world
current_camera.matrix_local = mathutils.Matrix.Identity(4)

current_rig.keyframe_insert(data_path="location", frame=bpy.context.scene.frame_current)
current_rig.keyframe_insert(data_path="rotation_euler", frame=bpy.context.scene.frame_current)

cam_filepath = Path(bpy.data.filepath).parent / "cameras.txt"
with open(cam_filepath, "w") as f:
    for rig in bpy.data.objects:
        if rig.name.startswith("CameraRigs/") and rig.name.count('/') == 1:
            f.write(f"{rig.name}\n")
            keyframe_data = {}
            for fcurve in rig.animation_data.action.fcurves:
                if fcurve.data_path in {"location", "rotation_euler"}:
                    axis = fcurve.array_index
                    for keyframe in fcurve.keyframe_points:
                        frame = int(keyframe.co.x)
                        value = keyframe.co.y
                        print(frame, fcurve.data_path, axis, value)
                        if frame not in keyframe_data:
                            keyframe_data[frame] = {
                                "location": [None, None, None],
                                "rotation_euler": [None, None, None]
                            }
                        keyframe_data[frame][fcurve.data_path][axis] = value
            for frame, transforms in sorted(keyframe_data.items()):
                loc = transforms["location"]
                rot = transforms["rotation_euler"]
                f.write(
                    f"{frame}, {loc[0]:.4f}, {loc[1]:.4f}, {loc[2]:.4f}, "
                    f"{rot[0]:.4f}, {rot[1]:.4f}, {rot[2]:.4f}\n"
                )
f.close()