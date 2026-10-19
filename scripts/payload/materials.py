from pathlib import Path

import astropy.units as u
from astropy.table import QTable


BOARD_DATA = Path("./board-data.ecsv")


materials = {
    "aluminum-6061": {
        "material": "aluminum-6061",
        "aluminum-6061": {
            "density": 2.70,  # g/cm3
            "components": {
                "Al": 0.9800,
                "Mg": 0.0100,
                "Si": 0.0050,
                "Cu": 0.0025,
                "Cr": 0.0025,
            },
        },
    },  # estimated from Wikipedia
    "tungsten": {
        "material": "G4_W",
    },
    "tungsten-alloy": {
        "material": "tungsten-alloy",
        "tungsten-alloy": {
            "density": 17,
            "components": {
                "W": 0.90,
                "Ni": 0.06,
                "Cu": 0.04,
            },
        },
    },
    "delrin": {
        "material": "delrin",
        "delrin": {
            "density": 1.415,
            "components": {"C": 0.400, "H": 0.067, "O": 0.533},
        },
    },
    "pla": {
        "material": "pla",
        "pla": {"density": 1.3, "components": {"C": 0.5000, "H": 0.0559, "O": 0.4441}},
    },
    "fr4": {
        "material": "fr4",
        "fr4": {
            "density": 1.86,  # this value is irrelevant
            "components": {
                "H": 0.077423 * 0.472,
                "C": 0.922577 * 0.472,
                "Si": 0.467435 * 0.528,
                "O": 0.532565 * 0.528,
            },
        },
    },  # from https://agenda.infn.it/event/15579/contributions/30406/attachments/21543/24545/GEANTSimulations2018_Raffa.pdf
    "lyso": {"material": "lyso", "type": "scintillator"},
    "yap": {"material": "yap", "type": "scintillator"},
}

# add this after declaration since it uses the PLA definition
materials["tungsten-filament"] = {
    "material": "tungsten-filament",
    "tungsten-filament": {
        "density": 7,
        "components": {
            "W": 0.9000,
            **{k: v * 0.1 for k, v in materials["pla"]["pla"]["components"].items()},
        },
    },
}


def _add_pcbs():
    """Add each PCB as a "material", which accounts for the FR4-copper
    composition of each board.
    """
    data = QTable.read(BOARD_DATA)
    for row in data:
        fr4_massfrac = row["FR4 mass (g)"] / row["total mass (g)"]
        copper_massfrac = row["copper mass (g)"] / row["total mass (g)"]
        board_density = (row["copper mass (g)"] + row["FR4 mass (g)"]) / row[
            "volume (mm3)"
        ] << u.g / u.cm**3
        material_name = str(row["board ()"])
        material_dict = {
            "material": material_name,
            str(row["board ()"]): {
                "density": board_density.to_value(u.g / u.cm**3),
                "components": {"Cu": copper_massfrac.value}
                | {
                    k: v * fr4_massfrac.value
                    for k, v in materials["fr4"]["fr4"]["components"].items()
                },
            },
        }
        materials[material_name] = material_dict

    # we don't know the content of the daqbox, so just assume
    materials["daqbox-board"] = materials["fc-board"]


_add_pcbs()
