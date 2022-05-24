#! /usr/bin/env/ python3

with open("file.txt", "w") as out:
    # out.seek(( 1024 * 1024)*100 - 1)
    for i in range(33000):
        out.write('Afik and Dvir Want to get 100!\n')
    out.write('PLEAS!\n')