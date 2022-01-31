package nz.bradley.neil.scandi.analysers;

import java.util.ArrayList;
import java.util.List;

public class Scope {
    public Scope parent;
    public final int depth;
    public final String value;
    public final boolean isDeclaration;
    public final List<Scope> children = new ArrayList<>();
    public final List<String> declarations = new ArrayList<>();
    public final List<String> usages = new ArrayList<>();

    public Scope(int depth, Scope parent, String value, boolean isDeclaration) {
        this.depth = depth;
        this.parent = parent;
        this.value = value;
        this.isDeclaration = isDeclaration;
    }

    public void debug(int depth) {
        System.out.println("DEBUG:" + "\t".repeat(depth) + value);
        System.out.println("DEBUG:" + "\t".repeat(depth + 1) + "DECLARATION:" + isDeclaration);
        System.out.println("DEBUG:" + "\t".repeat(depth + 1) + "DECLARATIONS:");
        declarations.forEach(o -> System.err.println("DEBUG:" + "\t".repeat(depth + 2) + o));
        System.out.println("DEBUG:" + "\t".repeat(depth + 1) + "USAGES:");
        usages.forEach(o -> System.err.println("DEBUG:" + "\t".repeat(depth + 2) + o));
        System.out.println("DEBUG:" + "\t".repeat(depth + 1) + "CHILDREN:");
        children.forEach(child -> child.debug(depth + 1));
    }
}
