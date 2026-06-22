from PIL import Image, ImageDraw, ImageFont
import textwrap

def create_screenshot(filename, term, extra_text, hint_text, is_revealed):
    # Pebble Time 2 (Emery) dimensions
    img = Image.new('RGB', (200, 228), color='black')
    draw = ImageDraw.Draw(img)

    try:
        font_term = ImageFont.truetype("Roboto-Black.ttf", 30)
        font_extra = ImageFont.truetype("Roboto-Bold.ttf", 22)
        font_hint = ImageFont.truetype("Roboto-Regular.ttf", 14)
    except IOError:
        font_term = ImageFont.load_default()
        font_extra = ImageFont.load_default()
        font_hint = ImageFont.load_default()

    # Calculate Y positions roughly matching our ui.c:
    # h = 228. term_y = h * 0.1 = 22.8
    # meaning_y = h * 0.35 + 5 = 84.8
    # bot_y = meaning_y + h * 0.55 = 84.8 + 125.4 = 210.2

    # Draw Term (Centered, White)
    term_bbox = draw.textbbox((0,0), term, font=font_term)
    term_w = term_bbox[2] - term_bbox[0]
    draw.text(((200 - term_w)/2, 23), term, font=font_term, fill="white")

    # Draw Extra (Left aligned, wrapped, Light Gray)
    # Wrap text to ~15 chars per line for 200px width at 22px font
    wrapped_extra = textwrap.fill(extra_text, width=17)
    if not is_revealed:
        extra_color = (180, 180, 180) # LightGray
    else:
        extra_color = (200, 200, 200) # Slightly brighter for meaning
        
    draw.multiline_text((15, 85), wrapped_extra, font=font_extra, fill=extra_color, spacing=4)

    # Draw Hint (Centered, Dark Gray)
    hint_bbox = draw.textbbox((0,0), hint_text, font=font_hint)
    hint_w = hint_bbox[2] - hint_bbox[0]
    draw.text(((200 - hint_w)/2, 210), hint_text, font=font_hint, fill=(100, 100, 100))

    img.save(filename)
    print(f"Generated {filename}")

# Generate Screenshot 1: Hidden
create_screenshot("screenshot1_hidden.png", "Ephemeral", "Press SELECT to reveal...", "[UP/DN: Skip]", False)

# Generate Screenshot 2: Revealed
create_screenshot("screenshot2_revealed.png", "Ephemeral", "Lasting for a very short time; transient.", "[HOLD SELECT: MASTERED]", True)

# Generate Screenshot 3: Another word
create_screenshot("screenshot3_revealed.png", "Ubiquitous", "Present, appearing, or found everywhere.", "[HOLD SELECT: MASTERED]", True)
