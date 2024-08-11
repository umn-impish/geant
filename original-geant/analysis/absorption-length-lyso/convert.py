import astropy.units as u
import astropy.constants as con
import numpy as np

# From the publication
CRYSTAL_LENGTH = 20 << u.cm

fn = 'lyso-self-abs.txt'
wavelength, transmission = np.loadtxt(fn, unpack=True)

transmission /= 100
wavelength <<= u.nm

energy = (con.h * con.c / wavelength).to_value(u.eV)
abs_length = (transmission * CRYSTAL_LENGTH).to_value(u.mm)

energy_sort_idxs = np.argsort(energy)

for i, e in enumerate(energy[energy_sort_idxs]):
    print(f'{e:.5f} * eV', end=', ')
    if (i + 1) % 7 == 0:
        print()
print()

for i, a in enumerate(abs_length[energy_sort_idxs]):
    print(f'{a:>6.2f} * mm', end=', ')
    if (i + 1) % 7 == 0:
        print()
print()
