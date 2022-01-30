package nz.bradley.neil.scandi;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;

public class Compiler {

    private static boolean debug;

    public static void main(String[] args) {
        if (args.length < 2) {
            usage();
            return;
        }

        List<String> inFiles;
        if ("DEBUG".equalsIgnoreCase(args[0])) {
            debug = true;
            inFiles = List.of(Arrays.copyOfRange(args, 1, args.length));
        } else {
            inFiles = List.of(args);
        }

        if (inFiles.isEmpty()) {
            usage();
            return;
        }

        List<String> assemblyFiles = compileToAssembler(inFiles);
    }

    private static void usage() {
        System.err.println("Usage: java Compiler [DEBUG] source1 [source2 ...]");
    }

    private static List<String> compileToAssembler(List<String> files) {
        List<String> compiled = new ArrayList<>();
        for (var file: files) {
            try {
                var relativeDotPath = getRelativeDotPath(file);
                var lines = stripCommentsAndBlanks(Files.readAllLines(Path.of(file)));
                var tokens = Token.tokenize(relativeDotPath, lines);
                var tree = Scope.buildScopeTree(tokens);
                if (verifyScopes(tree)) {
                    if (debug) {
                        System.err.println("SCOPES OK");
                    }
                }
            } catch (IOException e) {
                System.err.println("Error reading " + file);
                e.printStackTrace();
            }
        }
        return compiled;
    }

    private static String getRelativeDotPath(String file) {
        String filename = file.replaceAll("[/\\\\]", ".");
        if (filename.endsWith(".scandi")) {
            filename = filename.substring(0, filename.lastIndexOf(".scandi"));
        }
        while (filename.startsWith(".")) {
            filename = filename.replaceFirst(".", "");
        }
        return filename;
    }

    private static List<String> stripCommentsAndBlanks(List<String> lines) {
        List<String> stripped = new ArrayList<>();
        for (var line: lines) {
            if (line.contains("`")) {
                line = line.substring(0, line.indexOf("`"));
            }
            if (!line.isBlank()) {
                stripped.add(line);
            }
        }
        return stripped;
    }

    private static boolean verifyScopes(Scope root) {
        boolean status = true;
        return status;
    }
}
