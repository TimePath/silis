// SPDX-License-Identifier: AFL-3.0
import java.net.URI;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Spliterator;
import java.util.Spliterators;
import java.util.jar.JarOutputStream;
import java.util.stream.Collectors;
import java.util.stream.StreamSupport;
import java.util.zip.Deflater;
import java.util.zip.ZipEntry;

public class ExtractJRT {
    static public void main(String[] args) throws Throwable {
        var fs = FileSystems.getFileSystem(URI.create("jrt:/"));
        try (var jar = new JarOutputStream(Files.newOutputStream(Paths.get("rt.jar")))) {
            jar.setMethod(JarOutputStream.DEFLATED);
            jar.setLevel(Deflater.NO_COMPRESSION);
            Files.walk(fs.getPath("/")).forEach(it -> {
                if (!Files.isRegularFile(it)) {
                    return;
                }
                var list = StreamSupport.stream(
                        Spliterators.spliteratorUnknownSize(it.iterator(), Spliterator.NONNULL),
                        false
                )
                        .map(Path::toString)
                        .collect(Collectors.toList());
                if (!"modules".equals(list.remove(0))) {
                    throw new RuntimeException("Invalid JRT layout");
                }
                if (!"module-info.class".equals(list.get(list.size() - 1))) {
                    list.remove(0);
                }
                var entry = new ZipEntry(String.join("/", list));
                try {
                    jar.putNextEntry(entry);
                    jar.write(Files.readAllBytes(it));
                    jar.closeEntry();
                } catch (Throwable t) {
                    throw new RuntimeException(t);
                }
            });
        }
    }
}
