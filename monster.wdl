version 1.0

task BuildAndRunMonster {
    command <<<
        apt-get update -y && apt-get install g++ make git -y
        git clone https://github.com/MattMcL4475/MonsterGroup.git || exit 1
        cd MonsterGroup/MonsterGroup
        make || exit 1
        ./monster 1024
    >>>

    runtime {
        docker: "ubuntu:latest"
        vm_size: "Standard_M128"
    }

    output {
        String output = read_string(stdout())
    }
}

workflow Monster {
    call BuildAndRunMonster
}
