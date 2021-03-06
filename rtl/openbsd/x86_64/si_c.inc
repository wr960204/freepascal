{
    This file is part of the Free Pascal run time library.
    Copyright (c) 2019 by Free Pascal development team

    This file implements parts of the startup code for OpenBSD
    programs that link to the C library.

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{$asmmode gas}

var
  _etext: Byte; external name '_etext';
  _eprol: Byte; external name '_eprol';

procedure _mcleanup; external name '_mcleanup';
procedure atexit; external name 'atexit';
procedure monstartup; external name 'monstartup';
procedure _init; external name '_init';

procedure _FPC_proc___start; forward;

procedure _FPC_proc_start; assembler; nostackframe; public name '_start'; public name '__start';
  asm
    movq    %rbx,%r9
    movq    %rcx,%r8
    movq    %rdx,%rcx
    movq    (%rsp),%rdi
    leaq    16(%rsp,%rdi,8),%rdx
    leaq    8(%rsp),%rsi
    subq    $8,%rsp
    andq    $0xFFFFFFFFFFFFFFF0,%rsp
    addq    $8,%rsp
    jmp     _FPC_proc___start
  end;

procedure _FPC_proc_haltproc; forward;
function _strrchr(str: PChar; character: LongInt): PChar; forward;

procedure _FPC_proc___start; assembler; nostackframe; public name '___start';
  asm
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $64, %rsp
    movl    %edi, -20(%rbp)  { argc }
    movq    %rsi, -32(%rbp)  { argv }
    movq    %rdx, -40(%rbp)  { environment }
    movq    %rcx, -48(%rbp)  { ??? }
    movq    %r8, -56(%rbp)   { ??? }
    movq    %r9, -64(%rbp)   { ??? }
    movq    -40(%rbp), %rax
    movq    %rax, environ(%rip)
    movq    %rax,operatingsystem_parameter_envp(%rip)
    movq    -32(%rbp), %rax
    movq    (%rax), %rax
    movq    %rax, -8(%rbp)
    cmpq    $0, -8(%rbp)
    je      .L2
    movq    -8(%rbp), %rdi
    movl    $47, %esi
    call    _strrchr
    movq    %rax, __progname(%rip)
    movq    __progname(%rip), %rax
    testq   %rax, %rax
    jne     .L4
    movq    -8(%rbp), %rax
    movq    %rax, __progname(%rip)
    jmp     .L6
.L4:
    movq    __progname(%rip), %rax
    addq    $1, %rax
    movq    %rax, __progname(%rip)
.L6:
    leaq    __progname_storage(%rip), %rax
    movq    %rax, -16(%rbp)
    jmp     .L7
.L8:
    movq    __progname(%rip), %rcx
    movzbl  (%rcx), %edx
    movq    -16(%rbp), %rax
    movb    %dl, (%rax)
    addq    $1, -16(%rbp)
    leaq    1(%rcx), %rax
    movq    %rax, __progname(%rip)
.L7:
    movq    __progname(%rip), %rax
    movzbl  (%rax), %eax
    testb   %al, %al
    je      .L9
    leaq    __progname_storage+255(%rip), %rax
    cmpq    %rax, -16(%rbp)
    jb      .L8
.L9:
    leaq    __progname_storage(%rip), %rax
    movq    %rax, __progname(%rip)
    movq    -16(%rbp), %rax
    movb    $0, (%rax)
.L2:
    movq    _mcleanup@GOTPCREL(%rip), %rdi
    call    atexit
    movq    _etext@GOTPCREL(%rip), %rsi
    movq    _eprol(%rip), %rdi
    call    monstartup@plt
    movl    $0, %eax
    call    _init
    movq    environ(%rip), %rdx
    movq    -32(%rbp), %rsi
    movl    -20(%rbp), %edi
    movq    %rdi,operatingsystem_parameter_argc(%rip)
    movq    %rsi,operatingsystem_parameter_argv(%rip)
    movl    $0, %eax
    call    PASCALMAIN
    // movl    %eax, %edi
    // call    exit
    jmp     _FPC_proc_haltproc
  end;

procedure _FPC_proc_haltproc; assembler; nostackframe; public name '_haltproc';
  asm
    movq    $1,%rax
    movl    operatingsystem_result(%rip),%ebx
    pushq   %rbx
    call    .Lactualsyscall
    addq    $8,%rsp
    jmp     _FPC_proc_haltproc

.Lactualsyscall:
    int     $0x80
    jb      .LErrorcode
    xor     %rbx,%rbx
    ret
.LErrorcode:
    movq    %rax,%rbx
    movq    $-1,%rax
  end;

function _strrchr(str: PChar; character: LongInt): PChar; public name '_strrchr';
  begin
    _strrchr:=nil;
    repeat
      if str^=Chr(character) then
        _strrchr:=str;
      if str^<>#0 then
        Inc(str);
    until str^=#0;
  end;

procedure MD_EPROL_LABEL; assembler; nostackframe; public name '_eprol';
  asm
  end;
