def y_0 (y, x, i):

    a = 1 + 2
    b = 3.0
    c = a + b
    z = a + b
    x = c + z
    y = (3.0 + 2) + a + b * c - 2 + 4 * a + c / z

    a = 4
    b = 10000
    temp = 0
    if a > b:  # define the if condition 
        temp = b 
    else: 
        temp = a 
    i = 1
    gcd = 0
    while i < temp + 1:
        if a == i: 
            gcd = i

