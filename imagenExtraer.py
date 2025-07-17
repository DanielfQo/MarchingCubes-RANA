from PIL import Image
import os

nombres = {"blood","brain","duodenum","eye", "eyeRetna", "eyeWhite", "heart", "kidney", "ileum", "lIntestine", "liver", "lung", "muscle", "nerve", "skeleton", "stomach", "spleen"}

for nombre in nombres:

    ruta_tiff = f"zimagenT/{nombre}Masks.tiff"

    carpeta_salida = f"images/{nombre}"
    os.makedirs(carpeta_salida, exist_ok=True)

    with Image.open(ruta_tiff) as img:
        for i in range(img.n_frames):
            img.seek(i)
            capa = img.copy()
            nombre_salida = os.path.join(carpeta_salida, f"{i+1}.png")
            capa.save(nombre_salida)
