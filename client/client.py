import sys
import Image

def line((x,y),(x2,y2)):
    """Brensenham line algorithm"""
    steep = 0
    coords = []
    dx = abs(x2 - x)
    if (x2 - x) > 0: sx = 1
    else: sx = -1
    dy = abs(y2 - y)
    if (y2 - y) > 0: sy = 1
    else: sy = -1
    if dy > dx:
        steep = 1
        x,y = y,x
        dx,dy = dy,dx
        sx,sy = sy,sx
    d = (2 * dy) - dx
    for i in range(0,dx):
        if steep: coords.append((y,x))
        else: coords.append((x,y))
        while d >= 0:
            y = y + sy
            d = d - (2 * dx)
        x = x + sx
        d = d + (2 * dy)
    coords.append((x2,y2))
    return coords

def circle(radius): 
	x = 0 
	y = radius 
	switch = 3 - (2 * radius) 
	points = [] 
	while x <= y: 
		points.extend([(x,y),(x,-y),(-x,y),(-x,-y),\
					   (y,x),(y,-x),(-y,x),(-y,-x)]) 
		if switch < 0: 
			switch = switch + (4 * x) + 6 
		else: 
			switch = switch + (4 * (x - y)) + 10 
			y = y - 1
		x = x + 1 
	return points
	
im = Image.open("test.bmp")
pixels = im.load()

circle = circle(32)	

print "circle length: ", len(circle)
print circle
for pc in circle:
	linep = line((32,32), (32+pc[0], 32+pc[1]))
	for p in linep:
		pixels[p[0], p[1]] = 0;
	

im.show();