import okhttp3.*;
import java.io.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class PauseResumeDownloader {
    private static final String FILE_URL = "http://10.10.10.1/user/20250121/15H06M55S.data";
    private static final String OUTPUT_FILE = "downloaded.data";
    private static final int BUFFER_SIZE = 8192; // 8KB buffer size

    private OkHttpClient client;
    private AtomicBoolean isPaused;

    public PauseResumeDownloader() {
        client = new OkHttpClient();
        isPaused = new AtomicBoolean(false);
    }

    public void downloadFile() {
        File file = new File(OUTPUT_FILE);
        long downloadedBytes = file.exists() ? file.length() : 0;

        Request.Builder requestBuilder = new Request.Builder()
                .url(FILE_URL);

        // Add Range header if resuming
        if (downloadedBytes > 0) {
            requestBuilder.addHeader("Range", "bytes=" + downloadedBytes + "-");
        }

        Call call = client.newCall(requestBuilder.build());

        try (Response response = call.execute()) {
            if (!response.isSuccessful()) {
                System.out.println("Failed to download file: " + response);
                return;
            }

            // Check if server supports partial content
            if (downloadedBytes > 0 && response.code() != 206) {
                System.out.println("Server does not support partial content. Restarting download.");
                downloadedBytes = 0;
            }

            // Stream the response body to the file
            try (InputStream inputStream = response.body().byteStream();
                 RandomAccessFile outputFile = new RandomAccessFile(file, "rw")) {

                // Start writing from where the last download left off
                outputFile.seek(downloadedBytes);

                byte[] buffer = new byte[BUFFER_SIZE];
                int bytesRead;
                while ((bytesRead = inputStream.read(buffer)) != -1) {
                    if (isPaused.get()) {
                        System.out.println("Download paused. Progress saved.");
                        break;
                    }
                    outputFile.write(buffer, 0, bytesRead);
                    downloadedBytes += bytesRead;
                    System.out.println("Downloaded: " + downloadedBytes + " bytes");
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void pauseDownload() {
        isPaused.set(true);
    }

    public void resumeDownload() {
        isPaused.set(false);
        System.out.println("Resuming download...");
        downloadFile();
    }

    public static void main(String[] args) throws IOException {
        PauseResumeDownloader downloader = new PauseResumeDownloader();

        // Start the download in a separate thread
        Thread downloadThread = new Thread(downloader::downloadFile);
        downloadThread.start();

        // Simulate user pausing and resuming the download
        try {
            Thread.sleep(5000); // Let the download run for 5 seconds
            downloader.pauseDownload();

            Thread.sleep(3000); // Wait for 3 seconds before resuming
            downloader.resumeDownload();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
