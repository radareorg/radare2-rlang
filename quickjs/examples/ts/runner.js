const { exec, execFile } = require('child_process');
        console.log("PRERUN");
        exec("make");
        console.log("RUNED");
        /*

var yourscript = exec('make -C /Users/pancake/prg/radare2-rlang/quickjs/examples/ts',
        (error, stdout, stderr) => {
            console.log(stdout);
            console.log(stderr);
            if (error !== null) {
                console.log(`exec error: ${error}`);
            }
        });
        console.log("RUNNED");

        */