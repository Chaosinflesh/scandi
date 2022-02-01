package nz.bradley.neil.scandi;

import nz.bradley.neil.scandi.analysers.Lexical;
import nz.bradley.neil.scandi.analysers.Scope;
import nz.bradley.neil.scandi.analysers.Semantic;
import nz.bradley.neil.scandi.analysers.Syntax;

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
        List<String> warnings = new ArrayList<>();
        List<String> errors = new ArrayList<>();
        Scope root = new Scope(0, null, "", false);
        for (var file: files) {
            try {
                // 1. Get context
                var relativeDotPath = getRelativeDotPath(file);
                debug("relativeDotPath", relativeDotPath);

                // 2. Get code only
                var lines = stripCommentsAndBlanks(Files.readAllLines(Path.of(file)));
                debug("LINES", relativeDotPath, lines);

                // 3. Split symbols, identifiers and values
                var lexemes = Lexical.analyse(relativeDotPath, lines, warnings);
                debug("LEXEMES", relativeDotPath, lexemes);

                // 4. Check syntax
                if (Syntax.analyse(relativeDotPath, lexemes, warnings, errors)) {

                    // 5. Check semantics
                    var scope = Semantic.analyse(root, relativeDotPath, lexemes, warnings, errors);
                    debug("SCOPES", relativeDotPath, scope);
                }
            } catch (IOException e) {
                System.err.println("Error reading " + file);
                e.printStackTrace();
            }
        }
        if (!warnings.isEmpty()) {
            warnings.forEach(message -> System.err.println("WARNING:\t" + message));
            System.err.println("Warnings were generated, see log.");
        }
        if (!errors.isEmpty()) {
            errors.forEach(message -> System.err.println("ERROR:\t" + message));
            System.err.println("Errors were generated, no output was produced.");
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

    public static void debug(String context, String data) {
        if (debug) {
            System.out.println("DEBUG:" + "\t" + context + "\t" + data);
        }
    }

    public static void debug(String title, String context, List<String> data) {
        debug(title, context);
        data.forEach(line -> debug(context, line));
    }

    public static void debug(String title, String context, Scope scope) {
        debug(title, context);
        scope.debug(1);
    }
}
