#!/bin/sh
# A GDB that only pretends: it answers the commands of the machine interface the debugger
# plugin sends, with a program of one function stopped on its breakpoints, for the tests of
# the backend.  Each command it gets it says on stderr, as "mock: <command>;": what GDB writes
# there comes to the plugin without its newlines.
#
# A program named /missing is refused, as GDB refuses a file that is not there.

catch=0      # the number of the last catchpoint taken
continued=0  # how many times the program has gone on

while IFS= read -r line; do
    token=${line%%[!0-9]*}
    command=${line#"$token"}
    printf 'mock: %s;\n' "$command" >&2
    case $command in
    '-file-exec-and-symbols "/missing"')
        printf '%s^error,msg="/missing: No such file or directory."\n' "$token" ;;
    '-break-delete 99')
        printf '%s^error,msg="No breakpoint number 99."\n' "$token" ;;
    '-interpreter-exec console "quit"')
        exit 0 ;;
    -gdb-set* | -enable-pretty-printing | -file-exec-and-symbols* | -environment-cd* \
        | -inferior-tty-set* | -exec-arguments* | -break-delete* | -break-enable* \
        | -break-disable* | -break-condition* | -exec-abort)
        printf '%s^done\n' "$token" ;;
    '-break-insert -f "/src/a.c:7"' | '-break-insert -f -c "x > 1" "/src/a.c:7"')
        printf '%s^done,bkpt={number="1",type="breakpoint",line="7",fullname="/src/a.c"}\n' "$token" ;;
    # pending: a library not loaded yet; it comes on code when the program runs
    '-break-insert -f -d "/src/lib.c:9"')
        printf '%s^done,bkpt={number="2",type="breakpoint",pending="/src/lib.c:9"}\n' "$token" ;;
    '-break-insert -f "/src/none.c:1"')
        printf '%s^error,msg="No source file named /src/none.c."\n' "$token" ;;
    '-break-insert "*0x1001"')
        printf '%s^done,bkpt={number="5",type="breakpoint",addr="0x1001"}\n' "$token" ;;
    -break-insert*)
        printf '%s^done,bkpt={number="6",type="breakpoint"}\n' "$token" ;;
    -catch-throw | -catch-catch)
        catch=$((catch + 10))
        printf '%s^done,bkpt={number="%s",type="catchpoint"}\n' "$token" "$catch" ;;
    -exec-run)
        printf '%s^running\n' "$token"
        printf '*running,thread-id="all"\n'
        printf '~"Starting program: /bin/true\\n"\n'
        printf '@"program says hello\\n"\n'
        printf '=breakpoint-modified,bkpt={number="2",type="breakpoint",line="12"}\n'
        printf '*stopped,reason="breakpoint-hit",bkptno="1",frame={addr="0x1000",func="main",args=[],file="a.c",fullname="/src/a.c",line="7"}\n' ;;
    -exec-continue)
        continued=$((continued + 1))
        printf '%s^running\n*running,thread-id="all"\n' "$token"
        if [ "$continued" = 1 ]; then
            printf '*stopped,reason="breakpoint-hit",bkptno="%s",frame={addr="0x1100",func="__cxa_throw"}\n' "$catch"
        else
            printf '*stopped,reason="exited",exit-code="03"\n'
            printf '=thread-group-exited,id="i1",exit-code="03"\n'
        fi ;;
    -exec-next)
        printf '%s^running\n*running,thread-id="all"\n' "$token"
        printf '*stopped,reason="end-stepping-range",frame={addr="0x1004",func="main",fullname="/src/a.c",line="8"}\n' ;;
    -exec-finish)
        printf '%s^running\n*running,thread-id="all"\n' "$token"
        printf '*stopped,reason="function-finished",frame={addr="0x7f00",func="caller",fullname="/src/b.c",line="20"},gdb-result-var="$1",return-value="0"\n' ;;
    -exec-step)
        printf '%s^running\n*running,thread-id="all"\n' "$token"
        printf '*stopped,reason="signal-received",signal-name="SIGSEGV",signal-meaning="Segmentation fault",frame={addr="0x1008",func="main",fullname="/src/a.c",line="9"}\n' ;;
    -exec-until*)
        printf '%s^running\n*running,thread-id="all"\n' "$token"
        printf '*stopped,reason="location-reached",frame={addr="0x100c",func="main",fullname="/src/a.c",line="10"}\n' ;;
    -exec-interrupt)
        printf '%s^done\n' "$token" ;;
    -stack-list-frames)
        printf '%s^done,stack=[frame={level="0",addr="0x1000",func="main",file="a.c",fullname="/src/a.c",line="7"},frame={level="1",addr="0x7f00",func="__libc_start_call_main",from="/lib/libc.so.6"}]\n' "$token" ;;
    '-stack-list-variables --simple-values')
        printf '%s^done,variables=[{name="x",type="int",value="42"},{name="p",type="struct point"},{name="q"},{value="1"}]\n' "$token" ;;
    '-data-evaluate-expression "x+1"')
        printf '%s^done,value="43"\n' "$token" ;;
    -data-evaluate-expression*)
        printf '%s^error,msg="No symbol \\"nope\\" in current context."\n' "$token" ;;
    -data-list-register-names)
        printf '%s^done,register-names=["rax","rbx","","rip","eflags","st0","st1","xmm0"]\n' "$token" ;;
    -data-list-register-values*)
        printf '%s^done,register-values=[{number="0",value="0x1"},{number="1",value="0x2"},{number="3",value="0x1000"},{number="4",value="0x246"},{number="99",value="0x0"}]\n' "$token" ;;
    '-data-disassemble -a 0x1000 -- 4')
        printf '%s^done,asm_insns=[src_and_asm_line={line="7",file="a.c",fullname="/src/a.c",line_asm_insn=[{address="0x1000",func-name="main",offset="0",inst="push %%rbp"},{address="0x1001",func-name="main",offset="1",inst="mov %%rsp,%%rbp"}]},src_and_asm_line={line="8",file="a.c",fullname="/src/a.c",line_asm_insn=[{address="0x1004",func-name="main",offset="4",inst="nop"}]}]\n' "$token" ;;
    '-data-disassemble -a 0x2000 -- 4')
        printf '%s^error,msg="No function contains specified address."\n' "$token" ;;
    '-data-disassemble -s 0x2000 -e 0x2000+256 -- 4')
        printf '%s^done,asm_insns=[{address="0x2000",inst="nop"},{address="0x2001",inst="ret"}]\n' "$token" ;;
    '-var-create - * "p"')
        printf '%s^done,name="var1",numchild="2",value="{...}",type="struct point",has_more="0"\n' "$token" ;;
    '-var-create - * "x"')
        printf '%s^done,name="var2",numchild="0",value="42",type="int",has_more="0"\n' "$token" ;;
    '-var-list-children --all-values "var1"')
        printf '%s^done,numchild="2",children=[child={name="var1.x",exp="x",numchild="0",value="1",type="int"},child={name="var1.y",exp="y",numchild="0",value="2",type="int"}],has_more="0"\n' "$token" ;;
    -var-delete*)
        printf '%s^done,ndeleted="1"\n' "$token" ;;
    -interpreter-exec*)
        printf '~"words of the console\\n"\n%s^done\n' "$token" ;;
    -gdb-exit)
        printf '%s^exit\n' "$token"
        exit 0 ;;
    *)
        printf '%s^error,msg="Undefined MI command: %s"\n' "$token" "${command%% *}" ;;
    esac
done
