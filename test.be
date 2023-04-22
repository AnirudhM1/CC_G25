
let a : int = 5;
if a {
    let a : int = 6;
    dbg a;
}
else {
    dbg a;
    dbg 5;
}

if a {
    let a : int = 0;
    dbg a;

    if a{
        let a : int = 8;
        dbg a;
    }
    else {
        dbg 10;
    }
    dbg a;
}
else {
    dbg 5;
}

dbg a;