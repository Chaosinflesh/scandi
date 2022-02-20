package nz.bradley.neil.scandi.analysers;

import nz.bradley.neil.scandi.language.Token;

import java.util.ArrayList;
import java.util.List;

public class Scope {
    private Scope parent;
    private Token token;
    private final List<Scope> children = new ArrayList<>();

    private Scope() {}

    public Scope(
            Scope parent,
            Token token
    ) {
        this.parent = parent;
        this.token = token;
    }

    public Scope getParent() {
        return this.parent;
    }

    /**
     * Sets the parent of this Scope, and also adds itself to the
     * parent's children if not already present.
     * @param parent    The parent scope.
     */
    public void setParent(Scope parent) {
        this.parent = parent;
        if (!this.parent.children.contains(this)) {
            this.parent.children.add(this);
        }
    }

    public Token getToken() {
        return this.token;
    }

}
