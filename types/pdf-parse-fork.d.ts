declare module 'pdf-parse-fork' {
  interface PDFData {
    numpages: number;
    numrender: number;
    info: any;
    metadata: any | null;
    text: string;
    version: string;
  }

  function pdfParse(buffer: Buffer): Promise<PDFData>;
  
  export default pdfParse;
}

