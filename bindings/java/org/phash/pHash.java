package org.phash;
import java.io.*;

public class pHash
{

	public native static VideoHash videoHash(String file);
	public native static AudioHash audioHash(String file);
	public native static DCTImageHash dctImageHash(String file);
	public native static MHImageHash mhImageHash(String file);
	/**
	 * Marr-Hildreth image hash of a pre-decoded image held in memory.
	 *
	 * Useful for callers that already have pixel data (BufferedImage, an
	 * HTTP response body, etc.) and want to avoid round-tripping through a
	 * temp file. Produces the same hash as {@link #mhImageHash(String)}
	 * would for the same image.
	 *
	 * @param pixels   row-major interleaved pixel buffer; length must be
	 *                 width * height * channels bytes. 1-channel data is
	 *                 treated as luminance, 3-channel as RGB, 4-channel as
	 *                 RGBA with the alpha component discarded.
	 * @param width    image width in pixels
	 * @param height   image height in pixels
	 * @param channels 1, 3, or 4
	 * @return MHImageHash on success, or null on error (bad arguments,
	 *         allocation failure, etc.)
	 */
	public native static MHImageHash mhImageHashFromPixels(byte[] pixels, int width, int height, int channels);
	public native static double imageDistance(ImageHash hash1, ImageHash hash2);
	public native static double audioDistance(AudioHash hash1, AudioHash hash2);
	public native static double videoDistance(VideoHash hash1, VideoHash hash2, int threshold);
	private native static void pHashInit();
	private native static void cleanup();


	static {
        System.loadLibrary("pHash-jni");
		pHashInit();
	}

	public static MHImageHash[] getMHImageHashes(String d)
	{
		File dir = new File(d);
		MHImageHash[] hashes = null;
		if(dir.isDirectory())
		{
			File[] files = dir.listFiles();
			hashes = new MHImageHash[files.length];
			for(int i = 0; i < files.length; ++i)
			{
				MHImageHash mh = mhImageHash(files[i].toString());
				if(mh != null)
					hashes[i] = mh;
			}	

		}
		return hashes;

	}

	public static void main(String args[])
	{
        int i = 0;
        if(args[i].equals("-a"))
        {
            AudioHash audioHash1 = audioHash(args[1]);
            AudioHash audioHash2 = audioHash(args[2]);
            System.out.println("cs = " + audioDistance(audioHash1,audioHash2));
        }
        else if(args[i].equals("-dct"))
        {
            DCTImageHash imHash = dctImageHash(args[1]);
            DCTImageHash imHash2 = dctImageHash(args[2]);
            System.out.println("File 1: " + imHash.getFilename());
            System.out.println("File 2: " + imHash2.getFilename());

            System.out.println(imageDistance(imHash,imHash2));
        }
        else if(args[i].equals("-mh"))
        {
            MHImageHash imHash = mhImageHash(args[1]);
            MHImageHash imHash2 = mhImageHash(args[2]);
            System.out.println("File 1: " + imHash.getFilename());
            System.out.println("File 2: " + imHash2.getFilename());

            System.out.println(imageDistance(imHash,imHash2));

        }
        else if(args[i].equals("-v"))
        {
            VideoHash vHash = videoHash(args[1]);
            VideoHash vHash2 = videoHash(args[2]);
            System.out.println(videoDistance(vHash,vHash2, 21));
        }

        pHash.cleanup();
	}
}
