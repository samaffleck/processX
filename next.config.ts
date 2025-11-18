import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  /* config options here */
  experimental: {
    // Increase body size limit for PDF uploads (50MB)
    serverActions: {
      bodySizeLimit: '50mb',
    },
  },
};

export default nextConfig;
