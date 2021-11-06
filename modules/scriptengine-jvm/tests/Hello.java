class Hello {
    static boolean boolZero = false;
    static boolean boolOne = true;

    public static void main(String[] args) {
        if (boolZero) { System.out.println("Error: boolZero was true"); return; }
        if (boolOne); else { System.out.println("Error: boolOne was not true"); return; }
        hello();
    }

    public static void hello() {
        System.out.println("Hello, world!");
    }
}
