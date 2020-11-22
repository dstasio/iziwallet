import bpy
import struct

mesh = bpy.data.meshes[0]
indices = [mesh.polygons[x].loop_indices[y] for x in range(len(mesh.polygons)) for y in range(len(mesh.polygons[x].loop_indices))]

print('\n')
prev = -1

def float_to_int(f):
    return struct.unpack('i', struct.pack('f', f))[0]

hashes = []
for i in indices:
    v = mesh.vertices[mesh.loops[i].vertex_index]
    uv = mesh.uv_layers.active.data[i].uv
    n = v.normal
    v_hash  = float_to_int(v.co.x*3) + float_to_int(v.co.y*5) + float_to_int(v.co.z*7)
    n_hash  = float_to_int(  n.x*11) + float_to_int(  n.y*13) + float_to_int(  n.z*17)
    uv_hash = float_to_int( uv.x*19) + float_to_int( uv.y*23)
    hash = v_hash*27 + n_hash*29 + uv_hash*31
    if (hash in hashes):
        print("REPEATS: ", end='')
    else:
        hashes.append(hash)
    print("{:2d}: {: .2f}, {: .2f}, {: .2f},  {: .2f}, {: .2f}, {: .2f},  {:.2f}, {:.2f} -> {: d}".format(i, v.co.x, v.co.y, v.co.z, n.x, n.y, n.z, uv.x, uv.y, hash))
    
    prev = i