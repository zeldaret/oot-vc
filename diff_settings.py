#!/usr/bin/env python3

def apply(config, args):
    config['baseimg'] = '00000001.app'
    config['myimg'] = 'build/00000001.app'
    config['mapfile'] = 'build/00000001.map'
    config['source_directories'] = ['src', 'include']
    config['arch'] = 'ppc'
    config['map_format'] = 'mw' # gnu or mw
    config['mw_build_dir'] = 'build/' # only needed for mw map format
    config['makeflags'] = []
    config['objdump_executable'] = 'powerpc-eabi-objdump'
