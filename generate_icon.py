"""Generate the 28x28 launcher menu icon.

Design: a bold V (Vocab) over three progress dots -- the app's Leitner
bucket-dots motif. White on black, 1-bit friendly for b&w Pebbles.
"""
from PIL import Image, ImageDraw

img = Image.new('RGB', (28, 28), color='black')
draw = ImageDraw.Draw(img)

# Bold V, drawn as two parallel strokes per leg for crisp 1-bit rendering
draw.line([(5, 3), (14, 19)], fill='white', width=4)
draw.line([(23, 3), (14, 19)], fill='white', width=4)

# Three Leitner progress dots under the V: filled, filled, outline
draw.ellipse([(6, 23), (9, 26)], fill='white')
draw.ellipse([(12, 23), (15, 26)], fill='white')
draw.ellipse([(18, 23), (21, 26)], outline='white')

img.save('resources/images/menu_icon.png')
print("Icon generated: resources/images/menu_icon.png (28x28)")
