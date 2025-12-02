import struct
import zlib
import math

def create_png(width, height, pixels):
    """Creates a PNG file from raw pixel data."""
    png_data = b'\x89PNG\r\n\x1a\n'
    
    # IHDR chunk
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
    ihdr_crc = zlib.crc32(b'IHDR' + ihdr_data)
    png_data += struct.pack('>I', 13) + b'IHDR' + ihdr_data + struct.pack('>I', ihdr_crc)
    
    # IDAT chunk
    raw_data = b''
    for row in pixels:
        raw_data += b'\x00' + row
    
    compressed_data = zlib.compress(raw_data, 9)
    idat_crc = zlib.crc32(b'IDAT' + compressed_data)
    png_data += struct.pack('>I', len(compressed_data)) + b'IDAT' + compressed_data + struct.pack('>I', idat_crc)
    
    # IEND chunk
    iend_crc = zlib.crc32(b'IEND')
    png_data += struct.pack('>I', 0) + b'IEND' + struct.pack('>I', iend_crc)
    
    return png_data

def is_pixel_on(digit, x, y):
    """Returns true if the pixel at (x,y) should be on for the given digit."""
    # Center points for drawing
    cx, cy = 32, 32
    
    # 0 (۰) - a filled dot
    if digit == 0:
        return (x - cx)**2 + (y - cy)**2 < 8**2
    # 3 (۳) - Simplified two-hump shape
    elif digit == 3:
        if 18 <= y <= 22 and 22 <= x <= 42: return True
        if 32 <= y <= 36 and 22 <= x <= 42: return True
        if 20 <= x <= 24 and 20 <= y <= 34: return True
        if 40 <= x <= 44 and 20 <= y <= 34: return True
        return False
    # 4 (۴) - "W" like shape
    elif digit == 4:
        # Simplified using lines
        # Line from (20,44) to (32,20) and (32,20) to (44,44)
        if y > 20 and y < 44:
            if abs((y - 20) / (44 - 20) - (x - 32) / (20 - 32)) < 0.1: return True
            if abs((y - 20) / (44 - 20) - (x - 32) / (44 - 32)) < 0.1: return True
        return False
    # 5 (۵) - A circle/oval
    elif digit == 5:
        return abs(math.sqrt((x - cx)**2 + (y - cy)**2) - 16) < 2.5
    # 6 (۶) - Simplified reverse '2'
    elif digit == 6:
        if 18 <= y <= 22 and 22 <= x <= 42: return True
        if 20 <= x <= 24 and 22 <= y <= 48: return True
        return False
    # 7 (۷) - A 'V' shape
    elif digit == 7:
        if y > 20 and y < 44:
             if abs((y - 20) / (44 - 20) - (x - 32) / (44 - 32)) < 0.1: return True
             if abs((y - 20) / (44 - 20) - (x - 32) / (20 - 32)) < 0.1: return True
        return False
    # 8 (۸) - An inverted 'V' shape
    elif digit == 8:
        if y > 20 and y < 44:
             if abs((y - 44) / (20 - 44) - (x - 32) / (20 - 32)) < 0.1: return True
             if abs((y - 44) / (20 - 44) - (x - 32) / (44 - 32)) < 0.1: return True
        return False
    # 9 (۹) - A circle with a tail
    elif digit == 9:
        if abs(math.sqrt((x - cx)**2 + (y - cy - 5)**2) - 12) < 2.5: return True
        if 28 <= x <= 32 and 18 <= y <= 44: return True
        return False
    return False

def generate_digit_png(digit):
    """Generates a PNG file for a single Persian digit."""
    width, height = 64, 64
    bg_color = (52, 52, 52)  # Dark gray
    fg_color = (255, 255, 255) # White
    pixels = []

    for y in range(height):
        row_bytes = bytearray(width * 3)
        for x in range(width):
            color = fg_color if is_pixel_on(digit, x, y) else bg_color
            row_bytes[x*3:x*3+3] = color
        pixels.append(bytes(row_bytes))
    
    png_data = create_png(width, height, pixels)
    filename = f'persian_{digit}.png'
    with open(filename, 'wb') as f:
        f.write(png_data)
    print(f"Created {filename}")

if __name__ == '__main__':
    # We already have 1 and 2
    digits_to_create = [0, 3, 4, 5, 6, 7, 8, 9]
    for d in digits_to_create:
        generate_digit_png(d)
