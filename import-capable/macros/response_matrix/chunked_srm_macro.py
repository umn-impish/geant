import numpy as np
import generate_srm_macro as gsm
import os

de = 0.01
end = 20 + de
energy_bins = np.arange(de, end, de)
num_concurrent = 40

# how many bins we want to allocate to each run
stride = int(energy_bins.size / num_concurrent)

os.makedirs('srm_macro_files', exist_ok=True)

gsm.de = de
for i in range(0, energy_bins.size, stride):
    ea, eb = energy_bins[i], energy_bins[i+stride-1]
    gsm.ea, gsm.eb = ea, eb
    gsm.file_name = f'srm_macro_files/srm_{i}_{i+stride}.mac'
    gsm.write_macro()