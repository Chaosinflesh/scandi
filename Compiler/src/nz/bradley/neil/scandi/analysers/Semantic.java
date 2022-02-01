package nz.bradley.neil.scandi.analysers;

import java.util.List;

public class Semantic {

    public static Scope analyse(
            Scope root,
            String dotPath,
            List<String> lexemes,
            List<String> warnings,
            List<String> errors
    ) {
        Scope base = new Scope(1, root, dotPath, true);
        Scope current = base;

        // At this point, we know syntax is correct. So we should be able to
        // construct the tree from here.
        return base;
    }
}
