#!/bin/bash
sbatch -t 02:00:00 --ntasks=1 --cpus-per-task=1 --mem=24G --wrap "source ../.venv/bin/activate; python atmosphere_backscattering.py overhead-extended.asdf"
