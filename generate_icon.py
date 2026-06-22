from PIL import Image, ImageDraw, ImageFont
import os

img = Image.new('RGB', (28, 28), color='black')
draw = ImageDraw.Draw(img)

# Draw a minimalist V
draw.line([(6, 6), (14, 22)], fill='white', width=3)
draw.line([(14, 22), (22, 6)], fill='white', width=3)

img.save('resources/images/menu_icon.png')
print("Icon generated.")
