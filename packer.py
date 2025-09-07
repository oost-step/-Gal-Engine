import os
import zlib
import struct

KEY = 0x5A  # 简单异或密钥

def xor_encrypt(data: bytes, key: int) -> bytes:
    return bytes([b ^ key for b in data])

def pack_resources(base_dirs, output_file: str):
    """
    打包指定目录列表中的所有文件
    保留相对路径作为资源 key
    """
    with open(output_file, "wb") as out:
        files = []
        for base_dir in base_dirs:
            for root, _, filenames in os.walk(base_dir):
                for fn in filenames:
                    full_path = os.path.join(root, fn)
                    rel_path = os.path.relpath(full_path, ".").replace("\\", "/")

                    # 跳过输出文件本身 & 脚本自身
                    if rel_path == output_file or rel_path.endswith(".py"):
                        continue

                    with open(full_path, "rb") as f:
                        raw = f.read()
                        compressed = zlib.compress(raw)
                        encrypted = xor_encrypt(compressed, KEY)
                    files.append((rel_path, encrypted))
                    print(rel_path)

        out.write(struct.pack("<I", len(files)))
        for name, data in files:
            name_bytes = name.encode("utf-8")
            out.write(struct.pack("<I", len(name_bytes)))
            out.write(name_bytes)
            out.write(struct.pack("<I", len(data)))
            out.write(data)

    print(f"打包完成: {output_file}, 共 {len(files)} 个文件")

if __name__ == "__main__":
    # 只打包 assets/ 和 resources/ 目录
    pack_resources(["assets", "resources"], "resources.pak")
