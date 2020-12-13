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
    print(top_right, top_left, bottom_left, top_right, bottom_left, bottom_right)
