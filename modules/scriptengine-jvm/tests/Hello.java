class Hello {
    static boolean boolZero = false;
    static boolean boolOne = true;

    public static void main(String[] args) {
        if (boolZero) { System.out.println("Error: boolZero was true"); return; }
        if (boolOne); else { System.out.println("Error: boolOne was not true"); return; }
        var n = 1;
        for (var i = 0; i < 10; i += 1) {
            n = n + n;
        }
        if (!(n == 1024)) { System.out.println("Error: n != 1024"); return; }
        var instances = new Hello[]{new Hello()};
        if (instances.length != 1) { System.out.println("Error: instances.length != 1"); return; }
        var instance = instances[0];
        if (instance == null) { System.out.println("Error: instance != null"); return; }
        instance.setGreeting("Hello, world!");
        instance.hello();
    }

    String _greeting;

    void setGreeting(String greeting) {
        this._greeting = greeting;
    }

    String getGreeting() {
        return this._greeting;
    }

    void hello() {
        synchronized (this) {
            _hello(true, getGreeting());
        }
    }

    void _hello(boolean flag, String greeting) {
        System.out.println(greeting);
    }
}
