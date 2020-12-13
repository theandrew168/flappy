coords = []
for ry in range(-2, 3):
    for rx in range(2, -3, -1):
        x = rx / 4
        y = ry / 4
        coords.append((x,y))

for p in range(16):
    y, x = divmod(p, 4)
    top_right = coords[(y+1)*5 + x]
    top_left = coords[(y+1)*5 + x + 1]
    bottom_left = coords[y*5 + x + 1]
    bottom_right = coords[y*5 + x]
    line = '{{{{ {} }}}},'.format(', '.join('{: .2f}f' for _ in range(12)))
    line = line.format(
        top_right[0], top_right[1],
        top_left[0], top_left[1],
        bottom_left[0], bottom_left[1],
        top_right[0], top_right[1],
        bottom_left[0], bottom_left[1],
        bottom_right[0], bottom_right[1],
    )
    print(line)
