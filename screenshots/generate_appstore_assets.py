from PIL import Image, ImageDraw, ImageFont

def create_logo():
    # 144x144 Logo
    img = Image.new('RGB', (144, 144), color=(0, 0, 0)) # Pure black
    draw = ImageDraw.Draw(img)
    
    # Draw a bold V
    # We can draw it using lines or polygons
    poly = [(30, 20), (55, 20), (72, 90), (89, 20), (114, 20), (72, 120)]
    draw.polygon(poly, fill=(255, 255, 255))
    
    img.save('screenshots/appstore_logo.png')
    print("Generated 144x144 Logo")

def create_banner():
    # 720x320 Banner
    img = Image.new('RGB', (720, 320), color=(15, 15, 15)) # Very dark grey
    draw = ImageDraw.Draw(img)
    
    # Draw Logo on the left
    logo_img = Image.new('RGB', (144, 144), color=(0, 0, 0))
    logo_draw = ImageDraw.Draw(logo_img)
    poly = [(30, 20), (55, 20), (72, 90), (89, 20), (114, 20), (72, 120)]
    logo_draw.polygon(poly, fill=(255, 255, 255))
    img.paste(logo_img, (80, 88))
    
    # Load fonts
    try:
        font_title = ImageFont.truetype("screenshots/Roboto-Black.ttf", 64)
        font_subtitle = ImageFont.truetype("screenshots/Roboto-Regular.ttf", 32)
    except IOError:
        font_title = ImageFont.load_default()
        font_subtitle = ImageFont.load_default()
        
    # Draw Text
    draw.text((280, 100), "Vocab Learner", font=font_title, fill=(255, 255, 255))
    draw.text((285, 180), "Master words on your wrist.", font=font_subtitle, fill=(200, 200, 200))
    draw.text((285, 220), "Leitner Spaced Repetition", font=font_subtitle, fill=(150, 150, 150))
    
    img.save('screenshots/appstore_banner.png')
    print("Generated 720x320 Banner")

if __name__ == "__main__":
    create_logo()
    create_banner()
