#!/usr/bin/env python3

import argparse
import os
import subprocess

cmdLineParser = argparse.ArgumentParser()
cmdLineParser.add_argument('-N', type=int, required=True)
cmdLineParser.add_argument('--easicube', type=str, default='../build/easicube')
cmdLineParser.add_argument('--example_dir', type=str, default='../examples')
args = cmdLineParser.parse_args()

class Example(object):
    def __init__(self, name, frm, to):
        self.name = name
        self.frm = frm
        self.to = to

examples = [
    Example('0_constant', [0.0, 0.0, 0.0], [1.0, 1.0, 1.0]),
    Example('2_prem', [0.0, 0.0, 0.0], [3678298.565, 3678298.565, 3678298.565]),
    Example('3_layered_linear', [0.0, 0.0, -4000.0], [1.0, 1.0, 0.0]),
    Example('5_function', [0.0, 0.0, 0.0], [1.0, 25.0, 1.0]),
    Example('26_function', [-10.0, -1000.0, -100000.0], [10.0, 0.0, 100000.0]),
    Example('33_layered_constant', [0.0, -1000.0, 0.0], [1.0, 1000.0, 1.0]),
    Example('f_16_scec', [-5.0, 0.0, -1000.0], [1000.0, 1.0, 5.0]),
    Example('f_120_sumatra', [0.0, 0.0, -50000.0], [50000.0, 100000.0, 0.0])
]

print('example,npoints,tsetup,teval')

for example in examples:
    print(example.name, end=',', flush=True)
    spec = '{{from: [{}], to: [{}], N: [{}]}}'.format(
                ','.join(map(str, example.frm)),
                ','.join(map(str, example.to)),
                ','.join(map(str, [args.N]*len(example.frm)))
            )
    fileName = os.path.join(args.example_dir, example.name + '.yaml')
    subprocess.run([args.easicube, spec, fileName])
