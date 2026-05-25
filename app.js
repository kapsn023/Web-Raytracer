function init() {
    const button = document.getElementById("renderButton");
    button.addEventListener("click", function() {
        const sceneText = document.getElementById("sceneEditor").value;
        FS.writeFile("scene.txt", sceneText);

        const renderFn = Module.cwrap("render", "number", ["string"]);
        renderFn("scene.txt");

        const ppmData = FS.readFile("output.ppm", { encoding: "utf8" });
        const lines = ppmData.trim().split("\n");
        const header = lines[0].split(" ");
        const width = parseInt(header[1]);
        const height = parseInt(header[2]);

        const canvas = document.getElementById("renderCanvas");
        canvas.width = width;
        canvas.height = height;
        const ctx = canvas.getContext("2d");
        const imageData = ctx.createImageData(width, height);

        for (let i = 0; i < width * height; i++) {
            const pixel = lines[i + 1].split(" ");
            imageData.data[i * 4]     = parseInt(pixel[0]);
            imageData.data[i * 4 + 1] = parseInt(pixel[1]);
            imageData.data[i * 4 + 2] = parseInt(pixel[2]);
            imageData.data[i * 4 + 3] = 255;
        }

        ctx.putImageData(imageData, 0, 0);
    });
}

if (Module.calledRun) {
    init();
} else {
    Module.onRuntimeInitialized = init;
}
