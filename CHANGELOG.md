FALCON Changelog
================

# v1.1.0

## General
- Major refactoring of GUI
- Helper scripts for setting CPU governor and real-time permissions

## FalconEye
- Options for histogram (-H) and Short-cut decoding (-L)
- Option for subframe buffering (-W)
- Options for DCI format selection (-s, -S, -T)

## GUI
- Improve usability
- Improve performance
- Personalization options for:
    - Histogram and Short-cut decoding
    - Subframe buffering
    - GUI repaint intervals (FPS)
    - Scrollback history and viewport
- Add RNTI table showing per-user statistics

## Miscellaneous
- Code cleaning

# v1.0.0

## General
- Add subframe buffers and worker thread to relax processing-time constraint (counteract sample overflows and sync loss)
- Add optional subframe skipping in live mode when all subframe buffers are exceeded
- Add tracking of most active DCI formats (update each 500ms)
- Add selective DCI search: try most frequent DCI formats first (primary_meta_formats)
- Improve short-cut-based DCI detection
- Improve histogram-based DCI detection
- Disable RNTI discovery during aggregation-level disambiguation
- Fix (multi) activation of evergreen RNTIs (e.g. SI- and P-RNTI)
- Fix output of active RNTIs (remove expired RNTIs from output)
- Add optional file wrap when reading IQ samples from file
- Fix+Add SIMD computation of power spectrum
- Improve data exchange with GUI (containers, consumers)

## FalconEye
- Add (colored) ASCII output for UL/DL allocations and power spectrum (-r, -R)
- Add flag to disable short-cut detection, only use to RA and histogram (-H)
- Extend DCI logs by raw (hex) DCI payload and length

## GUI
- Improve plot performance
- Fix entering frequency above 2.1GHz
- Fix waterfall plots remain empty until window resize
- Fix computation of cell-activity plots

## Miscellaneous
- Refactoring and code cleanup
- Migration from C to C++ (WIP)
- Add some comments for better code understanding
- Add CI tests (testdata from a private external repository)
- Fix confused compiler params for C/C++
- Extend README.md
- Add this CHANGELOG

# v0.0.0

- Initial release
