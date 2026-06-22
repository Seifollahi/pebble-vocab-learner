from PIL import Image, ImageDraw

def create_48_icon():
    img = Image.new('RGB', (48, 48), color=(0, 0, 0)) # Pure black background
    draw = ImageDraw.Draw(img)
    
    # Draw a bold V scaled for 48x48
    poly = [(10, 7), (18, 7), (24, 30), (30, 7), (38, 7), (24, 40)]
    draw.polygon(poly, fill=(255, 255, 255))
    
    img.save('screenshots/appstore_icon_48.png')
    print("Generated 48x48 Icon")

if __name__ == "__main__":
    create_48_icon()
