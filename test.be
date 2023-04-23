fun add_if_a_is_one_else_sub(a : long, b : int) : long {
    if a - 1 {
        ret a - b;
    } else {
        ret a + b;
    }
}


fun main() : int {
    dbg add_if_a_is_one_else_sub(1, 40);
    dbg add_if_a_is_one_else_sub(100, 30);
}