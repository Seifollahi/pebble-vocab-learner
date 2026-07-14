"""Generate the 28x28 launcher menu icon.

Design: a bold V (Vocab) over three progress dots -- the app's Leitner
bucket-dots motif. White on black, 1-bit friendly for b&w Pebbles.
"""
from PIL import Image, ImageDraw

img = Image.new('RGB', (25, 25), color='black')
draw = ImageDraw.Draw(img)

# Bold V, drawn as two parallel strokes per leg for crisp 1-bit rendering
draw.line([(4, 2), (12, 17)], fill='white', width=4)
draw.line([(20, 2), (12, 17)], fill='white', width=4)

# Three Leitner progress dots under the V: filled, filled, outline
draw.ellipse([(5, 20), (8, 23)], fill='white')
draw.ellipse([(11, 20), (14, 23)], fill='white')
draw.ellipse([(17, 20), (20, 23)], outline='white')

img.save('resources/images/menu_icon.png')
print("Icon generated: resources/images/menu_icon.png (28x28)")
