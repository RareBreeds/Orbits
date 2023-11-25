module.exports = {
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
