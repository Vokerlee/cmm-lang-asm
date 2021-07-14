"frontend_cmm.exe" program.cmm       program.tree
"optimizer.exe"    program.tree      program_optm.tree
"backend.exe"      program_optm.tree program.vasm

"asm.exe"  program.vasm program.ncpu
"nCPU.exe" program.ncpu

pause