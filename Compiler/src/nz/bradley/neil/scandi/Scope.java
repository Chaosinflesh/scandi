package nz.bradley.neil.scandi;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicReference;

public record Scope(
        int depth,
        Scope parent,
        boolean isDeclaration,
        List<Scope> children,
        List<String> declarations,
        List<String> usages
) {
    public static Scope buildScopeTree(List<Token> tokens) {
        Scope root = new Scope(0, null, true, new ArrayList<>(), new ArrayList<>(), new ArrayList<>());
        AtomicReference<Scope> current = new AtomicReference<>(root);
        tokens.forEach(token -> {
            // TODO
        });
        return root;
    }
}
