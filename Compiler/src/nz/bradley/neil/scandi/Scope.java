package nz.bradley.neil.scandi;

import nz.bradley.neil.scandi.analysers.Lexe;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

public record Scope(
        int depth,
        Scope parent,
        boolean isDeclaration,
        List<Scope> children,
        List<String> declarations,
        List<String> usages
) {
    public static Scope buildScopeTree(List<Lexe> lexicalAnalyses) {
        Scope root = new Scope(0, null, true, new ArrayList<>(), new ArrayList<>(), new ArrayList<>());
        AtomicReference<Scope> current = new AtomicReference<>(root);
        lexicalAnalyses.forEach(lexe -> {
            // TODO
        });
        return root;
    }
}
