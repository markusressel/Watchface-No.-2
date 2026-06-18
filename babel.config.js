module.exports = function(api) {
  api.cache(true);
  const isRelease = process.env.PEBBLE_RELEASE === '1';
  return {
    presets: [
      [
        "@babel/preset-env",
        {
          targets: {
            ie: "9"
          }
        }
      ]
    ],
    plugins: [
      function({ types: t }) {
        return {
          visitor: {
            Identifier(path) {
              if (path.node.name === '__PEBBLE_RELEASE__') {
                path.replaceWith(t.booleanLiteral(isRelease));
              }
            }
          }
        };
      }
    ]
  };
};
