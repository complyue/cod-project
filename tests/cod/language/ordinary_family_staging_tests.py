#!/usr/bin/env python3
"""Ordinary callable families share native bodies across runtime installations."""
from pathlib import Path
import symbolic_family_staging_tests as runner

runner.SOURCE = runner.SOURCE.replace('symbol @f:proc(u64)->u64;', '')
runner.SOURCE = runner.SOURCE.replace('proc @f', 'func select')
runner.SOURCE = runner.SOURCE.replace('effect @f;', 'held<-select;')
runner.SOURCE = runner.SOURCE.replace('held<-select;', 'func held:func(u64)->u64=select;', 1)
runner.SOURCE = runner.SOURCE.replace('behave @f', 'held')
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
