package nz.bradley.neil.scandi.language;

public record Token(
        Type type,
        String value,
        String file,
        int line,
        int position
) {
    public enum Type {
        // 2 char symbols
        BINARY("__"),
        VARIABLE_RAISED("$$"),
        FUNCTION_RAISED("@@"),
        NULL("()"),
        VARARG("[]"),
        EQUALS_REF("?:"),
        LESS_EQUAL("?<"),
        GREATER_EQUAL("?>"),
        SHIFT_LEFT("<-"),
        SHIFT_RIGHT("->"),
        SHIFT_RIGHT_SIGNED(">>"),

        // 1 char symbols
        WITH_OPEN("{"),
        WITH_CLOSE("}"),
        LABEL("\\"),
        ADDRESS("_"),
        VARIABLE("$"),
        FUNCTION("@"),
        HEX("#"),
        STRING("\""),
        COUNT("!"),
        DOT("."),
        INDEX_OPEN("["),
        INDEX_CLOSE("]"),
        NEG_OPEN("("),
        NEG_CLOSE(")"),
        COPY("="),
        COPY_REF(":"),
        EQUALS("?"),
        LESS("<"),
        GREATER(">"),
        COMPLEMENT("~"),
        AND("&"),
        OR("|"),
        XOR("^"),
        ADD("+"),
        SUBTRACT("-"),
        MULTIPLY("*"),
        DIVIDE("/"),
        MODULUS("%"),

        // 0 char symbols
        DEPTH(";"),
        NUMBER("#!"),   // This doesn't exist within the language.
        IDENTIFIER("$!");   // This doesn't exist within the language.

        final String symbol;

        Type(String symbol) {
            this.symbol = symbol;
        }

        public static Type of(String value) {
            if (value != null && !value.isBlank()) {
                for (var type : Type.values()) {
                    if (value.startsWith(type.symbol)) {
                        return type;
                    }
                }
                if (isNumber(value)) {
                    return NUMBER;
                } else if (isIdentifier(value)) {
                    return IDENTIFIER;
                }
            }
            throw new IllegalArgumentException("Unknown Type: '" + value + "'");
        }

        public String getSymbol() {
            return symbol;
        }

    }

    public static boolean isHexadecimal(String isHex) {
        return isHex != null && isHex.matches("\\p{XDigit}+");
    }


    public static boolean isNumber(String isNo) {
        return isNo != null && isNo.matches("(\\p{Digit})+,?(\\p{Digit})*.*");
    }


    public static boolean isIdentifier(String isId) {
        return isId != null && !isId.isBlank() && Character.isAlphabetic(isId.codePointAt(0));
    }

    public static boolean isAssignment(Token token) {
        return switch (token.type()) {
            case COPY, COPY_REF -> true;
            default -> false;
        };
    }

    public static boolean isComparison(Token token) {
        return switch (token.type()) {
            case EQUALS, EQUALS_REF, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL -> true;
            default -> false;
        };
    }

    public static boolean isDeclaration(Token token) {
        return switch (token.type()) {
            case VARIABLE, VARIABLE_RAISED, FUNCTION, FUNCTION_RAISED, LABEL -> true;
            default -> false;
        };
    }

    public static boolean isOperator(Token token) {
        return switch (token.type()) {
            case COMPLEMENT, AND, OR, XOR, ADD, SUBTRACT, MULTIPLY, DIVIDE,
                    MODULUS, SHIFT_LEFT, SHIFT_RIGHT, SHIFT_RIGHT_SIGNED -> true;
            default -> false;
        };
    }
}
