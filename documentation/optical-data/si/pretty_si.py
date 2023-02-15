import numpy as np

def pretty(fmt, ar):
    for i, x in enumerate(ar):
        print(fmt.format(x), end=', ')
        if (i + 1) % 10 == 0: print()
    print()

e, n, k = np.loadtxt('si.tab', encoding='utf-8-sig', unpack=True)

print('e')
pretty('{:.2f}*eV', e)
print('n')
pretty('{:.2f}', n)
print('k')
pretty('{:.2f}', k)
