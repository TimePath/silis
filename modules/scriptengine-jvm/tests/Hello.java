class Hello {
    static boolean boolZero = false;
    static boolean boolOne = true;

    public static void main(String[] args) {
        if (boolZero) { System.out.println("Error: boolZero was true"); return; }
        if (boolOne); else { System.out.println("Error: boolOne was not true"); return; }
        var instance = new Hello();
        instance.hello();
    }

    void hello() {
        hello2();
    }

    void hello2() {
        System.out.println("Hello, world!");
    }
}
