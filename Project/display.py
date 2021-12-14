class display:

display_view = [];


def format(data, formatted_data):
    i = 0;
    for row in range(0, 32):
        for col in range(0,128):
            formatted_data[i] = data[row][col] & (data[row][col+1]<<1) & (data[row][col+2]<<2) & (data[row][col+3]<<3)
                                & (data[row][col+4]<<4) & (data[row][col+5]<<5) & (data[row][col+6]<<6) & (data[row][col+7]<<7)
            i++
        row+=8
