from PIL import Image, ImageDraw, ImageFont
import textwrap

def create_screenshot(filename, term, extra_text, hint_text, is_revealed, w, h, font_sizes):
    img = Image.new('RGB', (w, h), color='black')
    draw = ImageDraw.Draw(img)

    try:
        font_term = ImageFont.truetype("screenshots/Roboto-Black.ttf", font_sizes[0])
        font_extra = ImageFont.truetype("screenshots/Roboto-Bold.ttf", font_sizes[1])
        font_hint = ImageFont.truetype("screenshots/Roboto-Regular.ttf", font_sizes[2])
    except IOError:
        font_term = ImageFont.load_default()
        font_extra = ImageFont.load_default()
        font_hint = ImageFont.load_default()

    # Calculate Y positions
    term_bbox = draw.textbbox((0,0), term, font=font_term)
    term_w = term_bbox[2] - term_bbox[0]
    draw.text(((w - term_w)/2, h * 0.1), term, font=font_term, fill="white")

    # Extra
    wrap_width = int(w / (font_sizes[1] * 0.55))
    wrapped_extra = textwrap.fill(extra_text, width=wrap_width)
    extra_color = (180, 180, 180) if not is_revealed else (200, 200, 200)
    draw.multiline_text((10, h * 0.35), wrapped_extra, font=font_extra, fill=extra_color, spacing=4)

    # Hint
    hint_bbox = draw.textbbox((0,0), hint_text, font=font_hint)
    hint_w = hint_bbox[2] - hint_bbox[0]
    draw.text(((w - hint_w)/2, h * 0.85), hint_text, font=font_hint, fill=(100, 100, 100))

    img.save(filename)
    print(f"Generated {filename}")

# Aplite / Basalt / Diorite (144x168)
create_screenshot("screenshots/aplite_1.png", "Ephemeral", "Press SELECT to reveal...", "[UP/DN: Skip]", False, 144, 168, [20, 16, 12])
create_screenshot("screenshots/aplite_2.png", "Ephemeral", "Lasting for a very short time; transient.", "[HOLD SELECT: MASTERED]", True, 144, 168, [20, 16, 12])

# Chalk (180x180)
create_screenshot("screenshots/chalk_1.png", "Ephemeral", "Press SELECT to reveal...", "[UP/DN: Skip]", False, 180, 180, [24, 18, 12])
create_screenshot("screenshots/chalk_2.png", "Ephemeral", "Lasting for a very short time; transient.", "[HOLD SELECT: MASTERED]", True, 180, 180, [24, 18, 12])

