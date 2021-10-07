def parse_template_args(typename):
    if "<" not in typename:
        return
    csv = typename[typename.find("<") + 1: -1]
    depth = 0
    buf = ""
    for c in csv + ",":
        if c == ",":
            if depth:
                buf += c
            else:
                yield buf
                buf = ""
        elif c == " ":
            pass
        elif c == "<":
            buf += c
            depth += 1
        elif c == ">":
            buf += c
            depth -= 1
        else:
            buf += c
