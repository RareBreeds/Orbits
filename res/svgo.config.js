module.exports = {
  multipass: true,
  plugins: [
    {
      name: 'preset-default',
      params: {
        overrides: {
          cleanupNumericValues: {
            convertToPx: false,
          }
        }
      }
    }
  ]
};
